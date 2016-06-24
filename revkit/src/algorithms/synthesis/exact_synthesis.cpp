/* RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
 * Copyright (C) 2009-2011  The RevKit Developers <revkit@informatik.uni-bremen.de>
 *
 * This program is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this program.  If not, see <http://www.gnu.org/licenses/>.
 */

#include "exact_synthesis.hpp"

#include <cmath>
#include <fstream>
#include <iostream>

#include <boost/assign/std/vector.hpp>
#include <boost/assign/std/set.hpp>
#include <boost/lexical_cast.hpp>
#include <boost/bind.hpp>
#include <boost/format.hpp>

#include <core/utils/timer.hpp>
#include <core/functions/fully_specified.hpp>
#include <core/functions/add_gates.hpp>
#include <core/functions/copy_metadata.hpp>

#include <fmi/core.hpp>
#include <fmi/default_solver.hpp>
#include <fmi/vars.hpp>
#include <fmi/constant.hpp>
#include <fmi/assertions.hpp>
#include <fmi/solution.hpp>
#include <fmi/ops/cardinality-constraint.hpp>
#include <fmi/extract.hpp>
#include <fmi/groups.hpp>


namespace revkit  
{
  using namespace boost::assign;

  static bool constructCircuit (circuit& circ
      , fmi::default_solver& solver
      , unsigned depth
      , unsigned lines
      , std::vector < std::pair < fmi::bv, fmi::bv > > const& network)
  {

    circ.set_lines (lines);

    for (unsigned d = 0; d < network.size(); d++) 
    {
      fmi::bv const& controls = network[d].first;
      fmi::bv const& target   = network[d].second;

      std::vector < fmi::bv > vars;
      vars += controls, target;

      std::vector < boost::dynamic_bitset<> > assignments = fmi::get_assignment_vector (solver, vars);


      gate::line_container ctrls;
      gate::line t = -1;

      // check for 1's in control vector 
      for (unsigned c = 0; c < lines; c++)
      {
        if (assignments[0][c] == 1)
          ctrls += c;
      }

      t = assignments[1].to_ulong ();

      assert (find (ctrls.begin(), ctrls.end(), t) == ctrls.end());
      append_toffoli ( circ, ctrls, t );
    }

    return true;
  }

  struct is_value
  {
    bool operator() ( constant const& c )
    {
      return c;
    }
  };
   
  static void buildithLine (std::vector<std::pair<fmi::bv, fmi::bv> >& network
      , fmi::default_solver solver
      , binary_truth_table const& spec
      , binary_truth_table::const_iterator& line
      , std::vector < std::vector < fmi::bv > >& gates 
      , unsigned depth)
  {
    using fmi::_0;
    using fmi::_1;
    using fmi::_2;
    using fmi::_3;

    assert (gates.size() == 1); 
    
    unsigned lines = spec.num_inputs ();
    unsigned copies = std::distance ( spec.begin(), spec.end() );
    assert (pow (2.0, double(lines) ) == copies );

    unsigned pos = std::distance ( spec.begin(), line ); 


    fmi::bv current = gates [ 0 ] [ pos ]; 
    for (unsigned i = 0; i <= depth; i++)
    {
      fmi::bv hit = fmi::new_variable (solver, 1);
      fmi::bv next = fmi::new_variable (solver, lines);

      fmi::generate ( solver
          , _0 %= ( _1 & _2) == _2
          , hit, current, network[i].first ) ;

      fmi::bv zext = fmi::zero_extend ( solver, hit, lines - 1);
      fmi::generate (solver
          , _0 %= _1 ^ (_2 << _3)
          , next
          , current
          , zext
          , network[i].second ); 

      current = next; 
    }

     
    if ( std::find_if ( line->second.first, line->second.second, is_value() ) == line->second.second ) 
      return;

    bool hasDontCares = std::find ( line->second.first, line->second.second, 
        constant()) != line->second.second;

    std::string output, mask; 
    foreach (const binary_truth_table::value_type& out_bit, line->second)
    {
      output += boost::lexical_cast<std::string>(out_bit && *out_bit);
      mask   += boost::lexical_cast<std::string>(out_bit);
    }

    if (hasDontCares)
    {
      fmi::fmi_assertion ( solver, _1 == (_0 & _2)
          , current 
          , fmi::make_bin_constant (solver, output)
          , fmi::make_bin_constant (solver, mask ) );
    }
    else
      fmi::fmi_assertion (solver, _0 == _1
          , current
          , fmi::make_bin_constant (solver, output)); 
  }



  static bool incremental_line_synthesis (circuit& circ
      , binary_truth_table const& spec
      , properties::ptr settings
      , properties::ptr statistics 
      , timer<properties_timer>& timer )
  {
    using fmi::_0;
    using fmi::_1;
    using fmi::_2;
    using fmi::_3;

    fmi::default_solver solver = fmi::get_solver_instance ( get<std::string > ( settings, "solver", "MiniSAT") );
    unsigned max_depth         = get<unsigned> ( settings, "max_depth", 20u );
     
    unsigned lines = spec.num_inputs ();
    unsigned copies = std::distance ( spec.begin(), spec.end() );
    assert (pow (2.0, double(lines) ) == copies );

    //        // network.first = control lines, network.second = target 
    std::vector < std::pair < fmi::bv, fmi::bv >  > network;

    std::vector < fmi::bv > currentGate ( copies ); 

    fmi::bv one            = fmi::make_bin_constant (solver, "1");
    fmi::bv lines_constant = fmi::make_nat_constant (solver, lines, lines);
    
    std::vector < fmi::bv > nextGate ( copies ); 

    std::vector < std::vector < fmi::bv > > gates;

    std::vector< fmi::bv> gate; // input specification 
    for (binary_truth_table::const_iterator iter = spec.begin(); iter != spec.end(); ++iter)
    {
      std::string input;
      foreach (const binary_truth_table::value_type& in_bit, iter->first)
      {
        input += boost::lexical_cast<std::string>(*in_bit);
      }
      gate += fmi::make_bin_constant (solver, input); 
    }
    gates += gate; 


    int mainGroup = fmi::new_group (solver);
    fmi::store_group (solver, mainGroup); 
    for (unsigned i = 0; i < max_depth; ++i)
    {
      fmi::bv control = fmi::new_variable (solver, lines);
      fmi::bv target  = fmi::new_variable (solver, lines);

      fmi::bv ext      = fmi::zero_extend (solver, one, lines - 1);

      network += std::make_pair (control, target);
      fmi::fmi_assertion (solver 
         , (_0 | (_1 << _2)) != _0 
         , control, ext, target);

      fmi::fmi_assertion (solver
          , _0 < _1
          , target, lines_constant); 

    //   std::vector < fmi::bv > gate; 
    // //   for (unsigned k = 0; k < copies; k++) 
    // //     gate += fmi::new_variable (solver,  lines);
    //   gates += gate; 


      int specGroup = fmi::new_group ( solver ); 
      fmi::store_group (solver, specGroup);
      bool terminate = false; 

      binary_truth_table::const_iterator iter = spec.begin();
      do
      {
        //unsigned pos = std::distance ( spec.begin(), iter );


        buildithLine ( network, solver, spec, iter, gates, i); 

     //    // setup special step functions 
     //    if (pos % 2 == 0 && pos < copies) { ++iter; continue; } 

        fmi::solve_result result = fmi::solve (solver);
        iter++;

     //     std::ofstream mainGrpStream ("main.cnf");
     //     std::ofstream specGrpStream ("spec.cnf"); 

     //     assert (mainGrpStream);
     //     assert (specGrpStream); 
 
     //     fmi::dump_group (solver, specGroup, specGrpStream); 
     //     fmi::dump_group (solver, mainGroup, mainGrpStream);

        if (result == fmi::UNSAT) 
        {
          //std::cout << "UNSAT: " << pos << " " << copies << std::endl;
          fmi::delete_group (solver, specGroup);
          terminate = true;
           
          fmi::set_group (solver, mainGroup);
        }
        else  
        {
          assert (result == fmi::SAT); 
          if (iter == spec.end())
            return constructCircuit (circ, solver, network.size(), lines, network);
        }

        
      } while (!terminate); 
    }

    return false; 
  }

  static bool synthesis (circuit& circ
      , const binary_truth_table& spec
      , properties::ptr settings 
      , properties::ptr statistics
      , timer<properties_timer>& timer )
  {
    using fmi::_0;
    using fmi::_1;
    using fmi::_2;
    using fmi::_3;

    fmi::default_solver solver = fmi::get_solver_instance ( get<std::string>( settings, "solver", "MiniSAT" ) );
    unsigned max_depth         = get<unsigned>( settings, "max_depth", 20u );

    unsigned lines = spec.num_inputs ();
    unsigned copies = std::distance ( spec.begin(), spec.end() );
    assert (pow (2.0, double(lines) ) == copies );

    //        // network.first = control lines, network.second = target 
    std::vector < std::pair < fmi::bv, fmi::bv >  > network;
    //  

    int mainGroup = fmi::new_group (solver);
    fmi::store_group (solver, mainGroup);

    std::vector < fmi::bv > currentGate ( copies );

    for (binary_truth_table::const_iterator iter = spec.begin(); iter != spec.end(); ++iter)
    {
      unsigned pos = std::distance ( spec.begin(), iter );
      std::string input;
      foreach (const binary_truth_table::value_type& in_bit, iter->first)
      {
        input += boost::lexical_cast<std::string>(*in_bit);
      }

      currentGate[ pos ] = fmi::make_bin_constant (solver, input);
    }

    fmi::bv one            = fmi::make_bin_constant (solver, "1");
    fmi::bv lines_constant = fmi::make_nat_constant (solver, lines, lines);

    std::vector < fmi::bv > nextGate ( copies );
    for (unsigned i = 0; i < max_depth; i++)
    {
      //std::cout << "Depth: " << i << " Time: " << timer() << std::endl;
      fmi::bv control = fmi::new_variable (solver, lines);
      fmi::bv target  = fmi::new_variable (solver, lines);

      fmi::bv ext     = fmi::zero_extend (solver, one, lines - 1);

      network += std::make_pair (control, target);

      // target line cannot be a control line 
      fmi::fmi_assertion (solver, 
          (_0 | (_1 << _2)) != _0 , 
          control, ext, target);

      fmi::fmi_assertion (solver,
          _0 < _1,
          target, lines_constant);
        
       for (binary_truth_table::const_iterator iter = spec.begin(); iter != spec.end(); ++iter)
       {
         unsigned pos = std::distance ( spec.begin(), iter );

         nextGate [ pos ] = fmi::new_variable (solver, lines);
         fmi::bv hit      = fmi::new_variable (solver, 1);

         fmi::generate (solver
             , _0 %= (_1 & _2) == _2
             , hit, currentGate [ pos ] , control );

         fmi::bv zext = fmi::zero_extend (solver, hit, lines-1);

         fmi::generate (solver,
             _0 %= _1 ^ (_2 << _3),
             nextGate [ pos ], currentGate [ pos  ], zext, target);
       }

       currentGate = nextGate;

       int constraintGroup = fmi::new_group (solver);
       fmi::store_group (solver, constraintGroup);

       for (binary_truth_table::const_iterator iter = spec.begin(); iter != spec.end(); ++iter)
       {
         if ( std::find_if ( iter->second.first, iter->second.second, is_value() ) == iter->second.second )
           continue;

         unsigned pos = std::distance ( spec.begin(), iter );


         bool hasDontCares = std::find ( iter->second.first, iter->second.second, constant() ) != iter->second.second;

         // asserts the output  
         std::string output;
         std::string mask;
         foreach (const binary_truth_table::value_type& out_bit, iter->second)
         {
           output += boost::lexical_cast<std::string>(out_bit && *out_bit);
           mask   += boost::lexical_cast<std::string>(out_bit);
         }

         if (hasDontCares)
         {
           fmi::fmi_assertion ( solver, _1 == (_0 & _2), 
              currentGate [ pos ], fmi::make_bin_constant (solver, output), fmi::make_bin_constant ( solver, mask ) ); 
         }
         else
           fmi::fmi_assertion ( solver, _0 == _1, currentGate [ pos ], fmi::make_bin_constant (solver, output));
       }


       fmi::solve_result result = fmi::solve (solver);
       if (result == fmi::UNSAT) 
       {
         std::ofstream Astream ("A.cnf");
         std::ofstream Bstream ("B.cnf"); 

         fmi::dump_group ( solver, mainGroup, Astream);
         fmi::dump_group ( solver, constraintGroup, Bstream);
         fmi::delete_group ( solver, constraintGroup);
         fmi::set_group (solver, mainGroup); 

         continue;
       }

       return constructCircuit (circ, solver, network.size(), lines, network);
    }

    return false;
  }

  bool exact_synthesis( circuit& circ
      , const binary_truth_table& spec
      , properties::ptr settings
      , properties::ptr statistics )
  {

    timer<properties_timer> t;

    if ( statistics )
    {
      properties_timer rt( statistics );
      t.start( rt );
    }
  
    bool r = false;
    if ( get<bool> ( settings, "spec_incremental", true ) )
      r = incremental_line_synthesis ( circ, spec, settings, statistics, t );
    else
      r = synthesis ( circ, spec, settings, statistics, t);

    if (r) 
    {
      copy_metadata ( spec, circ );
    }
    else
    {
      set_error_message (statistics, "Could not find a circuit within the predefined depth."); 
    }

    //std::cout << "Runtime: " << t() << std::endl;

    return r;
  }

  truth_table_synthesis_func exact_synthesis_func ( properties::ptr settings, properties::ptr statistics )
  {
    truth_table_synthesis_func f = boost::bind( exact_synthesis, _1, _2, settings, statistics );
    f.init ( settings, statistics );
    return f;
  }

}

