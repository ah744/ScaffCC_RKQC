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

#include "equivalence_check.hpp"

#include <core/circuit.hpp>
#include <core/utils/timer.hpp>
#include <core/target_tags.hpp>
#include <core/properties.hpp>
#include <algorithms/verification/verification.hpp>
#include <core/functions/copy_circuit.hpp>

// STL and TR1
#include <tr1/tuple>
#include <utility>
#include <vector>

#include <stdexcept>

#include <boost/foreach.hpp>
#include <boost/format.hpp>
#include <boost/bind.hpp>
#include <boost/assign/std/vector.hpp>
#include <boost/range/algorithm.hpp>

#include <fmi/default_solver.hpp>
#include <fmi/assertions.hpp>
#include <fmi/solution.hpp>
#include <fmi/core.hpp>
#include <fmi/vars.hpp>
#include <fmi/reduce.hpp>
#include <fmi/reduce-tags.hpp>
#include <fmi/integral_wrapper.hpp> // for index
#include <fmi/extract.hpp>
#include <fmi/constant.hpp>
#include <fmi/ops/op-tags.hpp>

#define foreach BOOST_FOREACH

namespace revkit
{
  using namespace std;

  namespace binary
  {
    using namespace boost::assign;

    using fmi::_0;
    using fmi::_1;
    using fmi::_2;
    using fmi::_3;
    using fmi::_4;


    fmi::bv model_toffoli( fmi::default_solver& solver, const gate& g, const fmi::bv& input )
    {
      assert( is_toffoli( g ) && "Not a toffoli gate" );
      unsigned n = bitsize( input );

      fmi::bv output = fmi::new_variable( solver, n );

      gate::line target = *g.begin_targets();

      vector<fmi::bv> controls;
      for ( gate::const_iterator iter = g.begin_controls(); iter != g.end_controls(); ++iter )
      {
        controls += fmi::extract( solver, input, *iter );
      }

      // create identity of all lines unless target line
      for ( unsigned i = 0; i < n; ++i )
      {
        fmi::bv outExtr = fmi::extract (solver, output, i);
        fmi::bv inExt   = fmi::extract (solver, input, i);

        if (target != i)
        {
          fmi::fmi_assertion ( solver,
              _0 == _1, outExtr, inExt);
        }
      }

      fmi::bv outT = fmi::extract (solver, output, target);
      fmi::bv inT  = fmi::extract (solver, input,  target);

      fmi::fmi_assertion (solver,
          _0 == (_1 ^ _2),
          outT, inT,  fmi::reduce<fmi::and_op>( solver, controls.begin(), controls.end() ) );

      return output;
    }

    fmi::bv model_fredkin ( fmi::default_solver& solver, gate const& g, fmi::bv const& input )
    {
      assert ( is_fredkin (g) && "Not a fredkin gate");

      unsigned n = bitsize ( input );
      fmi::bv output = fmi::new_variable ( solver, n );

      std::vector<gate::line> targets;
      for (unsigned i = 0; i < n; i++)
      {
        if ( std::find ( g.begin_targets(), g.end_targets(), i ) == g.end_targets() )
        {
          fmi::fmi_assertion ( solver
             , _0 == _1
             , fmi::extract (solver, output, i)
             , fmi::extract (solver, input,  i) );
        }
        else
        {
          targets += i;
        }
      }
      assert (targets.size() == 2);

      std::vector < fmi::bv > sourceTargets;
      sourceTargets += fmi::extract (solver, input, targets[0]);
      sourceTargets += fmi::extract (solver, input, targets[1]);

      fmi::bv select;
      if (std::distance (g.begin_controls(), g.end_controls()) == 0)
      {
        select = fmi::make_bin_constant (solver, "1");
      }
      else
      {
        std::vector < fmi::bv > controls;

        for ( gate::const_iterator iter = g.begin_controls(); iter != g.end_controls(); ++iter )
        {
          controls += fmi::extract( solver, input, *iter );
        }

        select =  fmi::reduce<fmi::and_op>( solver, controls.begin(), controls.end() );
      }

      std::vector < fmi::bv > destTarget;
      destTarget += fmi::extract (solver, output, targets[0]);
      destTarget += fmi::extract (solver, output, targets[1]);

//       fmi::bv first = fmi::build_ite ( solver, select, sourceTargets[1], sourceTargets[0] );

      fmi::fmi_assertion ( solver, _0 == _1,
          destTarget[0], fmi::build_ite ( solver, select, sourceTargets[1], sourceTargets[0] ) );

      fmi::fmi_assertion ( solver, _0 == _1,
          destTarget[1], fmi::build_ite ( solver, select, sourceTargets[0], sourceTargets[1] ) );


      return output;
    }

    fmi::bv model_gate (fmi::default_solver& solver, const gate& g, const fmi::bv& input)
    {
      assert( !solver.empty() && "No solver specified" );

      if ( is_toffoli( g ) )
        return model_toffoli( solver, g, input );
      else if ( is_fredkin ( g ) )
        return model_fredkin ( solver,  g, input );

      std::cerr << "Warning: Gate type other than Toffoli and Fredkin is not supported yet. " << std::endl;
      return input;
    }
  }

  fmi::bv model_miter (fmi::default_solver& solver
      , const fmi::bv& lhs
      , const fmi::bv& rhs)
  {
    assert( !solver.empty() && "No solver specified." );
    assert( fmi::bitsize( lhs ) == fmi::bitsize( rhs ) );

     return boost::fusion::at_c<0>( fmi::generate (solver, fmi::_0 != fmi::_1, lhs, rhs ) );
  }

  static void constraintConstantInputs (fmi::default_solver& solver, circuit const& circ, fmi::bv const& input)
  {
    vector < constant > const& consts = circ.constants();
    for (vector < constant >::const_iterator iter = consts.begin(); iter != consts.end(); ++iter)
    {
      constant const& c = *iter;
      if (c)
      {
        unsigned pos = distance ( consts.begin(), iter );

        fmi::bv v = fmi::extract ( solver, input, pos );
        fmi::fmi_assertion (solver,
            fmi::_0 == fmi::_1,
            v, fmi::make_bin_constant (solver, *c ? "1" : "0" ) );
      }
    }
  }

  static map<string, string> get_input_default_mapping (circuit const& spec, circuit const& impl)
  {
    std::vector<std::string> const& specIns = spec.inputs();
    std::vector<std::string> const& implIns = impl.inputs();

    std::vector < constant > const& specConsts = spec.constants ();
    std::vector < constant > const& implConsts = impl.constants ();

    map  < string, string > mapping;
    for (unsigned i = 0; i < specIns.size(); i++)
    {
      if (!specConsts[i])
      {
        std::string const& specVar = specIns[i];

        std::vector<std::string>::const_iterator iter = find (implIns.begin(), implIns.end(), specVar);

        // FIXME iter == end()??
        if (iter != implIns.end())
        {
          unsigned pos = distance ( implIns.begin(), iter );

          if (!implConsts[pos])
          {
            mapping.insert (make_pair (specVar, specVar));
          }
        }
        else
        {
	  //mapping error
	  throw std::out_of_range("input mapping of the two circuits does't match.");
	  //idea
	  //mapping.insert (make_pair (specVar, implIns[i]));

        }
      }
    }

    return mapping;
  }

  static map<string, string> get_output_default_mapping (circuit const& spec, circuit const& impl)
  {
    std::vector<std::string> const& specOuts = spec.outputs();
    std::vector<std::string> const& implOuts = impl.outputs();

    std::vector < bool > const& specGarbages = spec.garbage ();
    std::vector < bool > const& implGarbages = impl.garbage ();

    map  < string, string > mapping;
    for (unsigned i = 0; i < specOuts.size(); i++)
    {
      if (!specGarbages[i])
      {
        std::string const& specVar = specOuts[i];

        std::vector<std::string>::const_iterator iter = find (implOuts.begin(), implOuts.end(), specVar);

        // FIXME iter == end()??
        if (iter != implOuts.end())
        {
          unsigned pos = distance ( implOuts.begin(), iter );

          if (!implGarbages[pos])
          {
            mapping.insert (make_pair (specVar, specVar));
          }
        }
        else
        {
	  //mapping error
	  throw std::out_of_range("output mapping of the two circuits does't match.");
	  //idea
          //mapping.insert (make_pair (specVar, implOuts[i]));
        }
      }
    }

    return mapping;
  }


  //
  // 0 -> input
  // 1 -> spec output
  tr1::tuple<fmi::bv, fmi::bv> build_ec_model( const circuit& spec
      , const circuit& impl
      , properties::ptr settings
      , properties::ptr statistics
      , fmi::default_solver& solver )
  {
    fmi::bv specInput = fmi::new_variable( solver, spec.lines() );
    fmi::bv implInput = fmi::new_variable (solver, impl.lines() );

    constraintConstantInputs ( solver, spec, specInput );
    constraintConstantInputs ( solver, impl, implInput );

    fmi::bv lines = specInput;
    foreach ( const gate& aGate, spec )
    {
      lines = binary::model_gate( solver, aGate, lines );
    }
    fmi::bv specOutput = lines;

    lines = implInput;
    foreach ( const gate& aGate, impl )
    {
      lines = binary::model_gate( solver, aGate, lines );
    }
    fmi::bv implOutput = lines;

    map<string,string> inMapping  = get<map<string,string> > (settings, "input_mapping",  map<string,string>());
    map<string,string> outMapping = get<map<string,string> > (settings, "output_mapping", map<string,string>());

    if (inMapping.empty())
      inMapping = get_input_default_mapping ( spec, impl );
    if (outMapping.empty())
      outMapping  = get_output_default_mapping ( spec, impl );

    vector<string> const& specInputs = spec.inputs();
    vector<string> const& implInputs = impl.inputs();
    vector<string> const& specOutputs = spec.outputs();
    vector<string> const& implOutputs = impl.outputs();

    for (map<string,string>::const_iterator iter = inMapping.begin(); iter != inMapping.end(); ++iter)
    {
      string const& specIn = iter->first;
      string const& implIn = iter->second;

      unsigned specPos = distance ( specInputs.begin(), find ( specInputs.begin(), specInputs.end(), specIn) );
      unsigned implPos = distance ( implInputs.begin(), find ( implInputs.begin(), implInputs.end(), implIn) );

      fmi::bv implVar = fmi::extract (solver, implInput, implPos);
      fmi::bv specVar = fmi::extract (solver, specInput, specPos);

      fmi::fmi_assertion ( solver, fmi::_0 == fmi::_1, implVar, specVar);
    }

    pair < vector < fmi::bv >, vector < fmi::bv > > miter;
    for (map<string,string>::const_iterator iter = outMapping.begin(); iter != outMapping.end(); ++iter)
    {
      string const& specOut = iter->first;
      string const& implOut = iter->second;

      unsigned specPos = distance ( specOutputs.begin(), find ( specOutputs.begin(), specOutputs.end(), specOut) );
      unsigned implPos = distance ( implOutputs.begin(), find ( implOutputs.begin(), implOutputs.end(), implOut) );

      fmi::bv implVar = fmi::extract (solver, implOutput, implPos);
      fmi::bv specVar = fmi::extract (solver, specOutput, specPos);

      miter.first.push_back (implVar);
      miter.second.push_back (specVar);
    }


    fmi::bv lhs = fmi::concat (solver, miter.first);
    fmi::bv rhs = fmi::concat (solver, miter.second);

    fmi::fmi_assertion( solver, model_miter( solver, lhs, rhs ) );

    return tr1::tuple<fmi::bv, fmi::bv> ( implInput, specOutput );
  }


  bool equivalence_check ( const circuit& spec_init
      , const circuit& impl_init
      , properties::ptr settings
      , properties::ptr statistics )

  {
    timer<properties_timer> t;

    if ( statistics )
    {
      properties_timer rt( statistics );
      t.start( rt );
    }

    fmi::default_solver solver  = fmi::get_solver_instance ( get<string>( settings, "solver", "MiniSAT" ) );
    unsigned max_counterexample = get<unsigned> (settings, "max_counterexample", 10u);
    equivalence_func preprocess = get<equivalence_func>( settings, "preprocess", equivalence_func() );

    if ( preprocess )
    {
      if ( preprocess( spec_init, impl_init ) )
      {
        boost::optional<bool> equivalent = preprocess.statistics()->get<boost::optional<bool> >( "equivalent" );

        if ( equivalent && !*equivalent )
        {
          statistics->set( "equivalent", false );
          return true;
        }
      }
    }

    //swap spec and impl if spec has more lines then impl
    circuit spec, impl;
    if(spec_init.lines() < impl_init.lines()){
      copy_circuit(spec_init, spec);
      copy_circuit(impl_init, impl); 
    }else{
      copy_circuit(spec_init, impl);
      copy_circuit(impl_init, spec); 
    }

    counterexample cex;
    try{

    fmi::bv input, specOutput;
    tr1::tie( input, specOutput ) = build_ec_model( spec, impl, settings, statistics, solver );

    fmi::solve_result s_result;

    vector<fmi::bv> vars;
    vars.push_back (input);
    vars.push_back (specOutput);

    do
    {
      s_result = fmi::solve (solver);

      // equivalent or no more countereaxmples exist
      if (s_result == fmi::UNSAT) break;

      vector<boost::dynamic_bitset<> > assignments = fmi::get_assignment_vector( solver, vars );
      fmi::add_blocking_clause( solver, input, assignments[0] );

      // Adjust input and output vector
      boost::dynamic_bitset<> cex_input( boost::count( impl.constants(), constant() ) );
      boost::dynamic_bitset<> cex_output( boost::count( spec.garbage(), false ) );

      unsigned j = 0u;
      for ( unsigned i = 0u; i < impl.lines(); ++i )
      {
        if ( !impl.constants().at( i ) )
        {
          cex_input.set( j++, assignments[0][i] );
        }
      }
      j = 0u;
      for ( unsigned i = 0u; i < spec.lines(); ++i )
      {
        if ( !spec.garbage().at( i ) )
        {
          cex_output.set( j++, assignments[1][i] );
        }
      }

      cex.push_back ( make_pair( cex_input, cex_output ) );
    } while ( s_result != fmi::UNSAT && cex.size() < max_counterexample);

    statistics->set ("equivalent", cex.empty() ? true : false);
    statistics->set ("counterexample", cex);
    
    }
    catch(const std::out_of_range& ex) {//input/output dismatch
      statistics->set ("equivalent", false);    
      statistics->set ("counterexample", cex);
      set_error_message(statistics, ex.what());
    }
    return true;

  }

  equivalence_func equivalence_check_func (properties::ptr settings, properties::ptr statistics )
  {
    equivalence_func f = boost::bind( equivalence_check, _1, _2, settings, statistics );
    f.init ( settings, statistics );
    return f;
  }
}

