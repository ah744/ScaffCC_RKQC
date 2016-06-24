# RevKit: A Toolkit for Reversible Circuit Design (www.revkit.org)
# Copyright (C) 2009-2011  The RevKit Developers <revkit@informatik.uni-bremen.de>
#
# This program is free software: you can redistribute it and/or modify
# it under the terms of the GNU General Public License as published by
# the Free Software Foundation, either version 3 of the License, or
# (at your option) any later version.
#
# This program is distributed in the hope that it will be useful,
# but WITHOUT ANY WARRANTY; without even the implied warranty of
# MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
# GNU General Public License for more details.
#
# You should have received a copy of the GNU General Public License
# along with this program.  If not, see <http://www.gnu.org/licenses/>.

from revkit_python import *

def error_msg( statistics ):
    "Returns the error message contained in statistics"
    return "Error: " + statistics.get_string( "error", "" )

def revkit_commands():
    "Prints a list of all data structures and commands available in RevKit"

    data_structures = dict()
    data_structures['1Data Structures'] = [ circuit, gate, binary_truth_table, properties ]
    data_structures['2Core Functions'] = [ append_circuit, prepend_circuit, insert_circuit, \
                                          append_cnot, append_not, append_toffoli, append_fredkin, append_v, append_vplus, append_peres, append_module, \
                                          prepend_cnot, prepend_not, prepend_toffoli, prepend_fredkin, prepend_v, prepend_vplus, prepend_peres, prepend_module, \
                                          insert_cnot, insert_not, insert_toffoli, insert_fredkin, insert_v, insert_vplus, insert_peres, insert_module, \
                                          add_line_to_circuit, circuit_to_truth_table, clear_circuit, copy_circuit, copy_metadata, control_lines, create_simulation_pattern, expand_circuit, extend_truth_table, \
                                          find_empty_lines, find_non_empty_lines, fully_specified, reverse_circuit, target_lines ]
    data_structures['3Core Functions (I/O)'] = [ create_image, print_circuit, print_statistics, read_realization, read_specification, read_pattern, read_pla, write_blif, write_verilog, write_realization, write_specification ]
    data_structures['4Core Functions (Utils)'] = [ costs, program_options ]
    data_structures['5Algorithms (Optimization)'] = [ adding_lines, line_reduction, lnn_optimization, window_optimization ]
    data_structures['6Algorithms (Simulation)'] = [ partial_simulation, simple_simulation, sequential_simulation ]
    data_structures['7Algorithms (Synthesis)'] = [ bdd_synthesis, embed_truth_table, esop_synthesis, exact_synthesis, kfdd_synthesis, quantum_decomposition, reed_muller_synthesis, swop, transformation_based_synthesis, transposition_based_synthesis ]
    data_structures['8Algorithms (Verification)'] = [ equivalence_check ]

    keys = data_structures.keys()
    keys.sort()

    for key in keys:
        print
        print( '  {0:s}'.format( key[1:] ) )
        print( '  {0:s}'.format( ''.join( map( lambda x: '-', range( 0, len( key[1:] ) ) ) ) ) )

        for i in data_structures[key]:
            doc = None
            if not i.__doc__ is None and len( i.__doc__.split( '\n' ) ) > 0:
                doc = i.__doc__.split( '\n' )[0]
            print( '   {0:22s} {1:s}'.format( i.__name__, doc ) )

### CORE/FUNCTIONS
def append_circuit( circ, src, controls = [] ):
    "Appends circuit src to circ controlled by controls"
    return py_append_circuit( circ, src, controls )

def prepend_circuit( circ, src, controls = [] ):
    "Prepends circuit src to circ controlled by controls"
    return py_prepend_circuit( circ, src, controls )

def insert_circuit( circ, pos, src, controls = [] ):
    "Inserts circuit src to circ controlled by controls"
    return py_insert_circuit( circ, pos, src, controls )

def append_cnot( circ, control, target ):
    "Appends a CNOT gate to circ"
    return py_append_cnot( circ, control, target )

def append_not( circ, target ):
    "Appends a NOT gate to circ"
    return py_append_not( circ, target )

def append_toffoli( circ, controls, target ):
    "Appends a Toffoli gate to circ"
    return py_append_toffoli( circ, controls, target )

def append_fredkin( circ, controls, target1, target2 ):
    "Appends a Fredkin gate to circ"
    return py_append_fredkin( circ, controls, target1, target2 )

def append_v( circ, control, target ):
    "Appends a quantum V gate to circ"
    return py_append_v( circ, control, target )

def append_vplus( circ, control, target ):
    "Appends a quantum V+ gate to circ"
    return py_append_vplus( circ, control, target )

def append_peres( circ, control, target1, target2 ):
    "Appends a Peres gate to circ"
    return py_append_peres( circ, control, target1, target2 )

def append_module( circ, name, controls, targets ):
    "Appends a module gate to circ"
    return py_append_module( circ, name, controls, targets )

def prepend_cnot( circ, control, target ):
    "Prepends a CNOT gate to circ"
    return py_prepend_cnot( circ, control, target )

def prepend_not( circ, target ):
    "Prepends a NOT gate to circ"
    return py_prepend_not( circ, target )

def prepend_toffoli( circ, controls, target ):
    "Prepends a Toffoli gate to circ"
    return py_prepend_toffoli( circ, controls, target )

def prepend_fredkin( circ, controls, target1, target2 ):
    "Prepends a Fredkin gate to circ"
    return py_prepend_fredkin( circ, controls, target1, target2 )

def prepend_v( circ, control, target ):
    "Prepends a quantum V gate to circ"
    return py_prepend_v( circ, control, target )

def prepend_vplus( circ, control, target ):
    "Prepends a quantum V+ gate to circ"
    return py_prepend_vplus( circ, control, target )

def prepend_peres( circ, control, target1, target2 ):
    "Prepends a Peres gate to circ"
    return py_prepend_peres( circ, control, target1, target2 )

def prepend_module( circ, name, controls, targets ):
    "Prepends a module gate to circ"
    return py_prepend_module( circ, name, controls, targets )

def insert_cnot( circ, pos, control, target ):
    "Inserts a CNOT gate to circ"
    return py_insert_cnot( circ, pos, control, target )

def insert_not( circ, pos, target ):
    "Inserts a NOT gate to circ"
    return py_insert_not( circ, pos, target )

def insert_toffoli( circ, pos, controls, target ):
    "Inserts a Toffoli gate to circ"
    return py_insert_toffoli( circ, pos, controls, target )

def insert_fredkin( circ, pos, controls, target1, target2 ):
    "Inserts a Fredkin gate to circ"
    return py_insert_fredkin( circ, pos, controls, target1, target2 )

def insert_v( circ, pos, control, target ):
    "Inserts a quantum V gate to circ"
    return py_insert_v( circ, pos, control, target )

def insert_vplus( circ, pos, control, target ):
    "Inserts a quantum V+ gate to circ"
    return py_insert_vplus( circ, pos, control, target )

def insert_peres( circ, pos, control, target1, target2 ):
    "Inserts a Peres gate to circ"
    return py_insert_peres( circ, pos, control, target1, target2 )

def insert_module( circ, pos, name, controls, targets ):
    "Inserts a module gate to circ"
    return py_insert_module( circ, pos, name, controls, targets )

def add_line_to_circuit( circ, inp, out, is_control = None, is_garbage = False ):
    """Adds a line to the circuit
     where inp and out are the labels for the in- and output and
     is_control and is_garbage determine whether the line has a constant input
     and/or a garbage output, respectively"""
    return py_add_line_to_circuit( circ, inp, out, is_control, is_garbage )

def clear_circuit( circ ):
    "Clears the circuit"
    return py_clear_circuit( circ )

def copy_circuit( src, dest ):
    """Copies circuit
     from src to dest"""
    return py_copy_circuit( src, dest )

def copy_metadata( base, circ, copy_inputs = True, copy_outputs = True, copy_constants = True, copy_garbage = True, copy_name = True, copy_inputbuses = True, copy_outputbuses = True, copy_statesignals = True, copy_modules = True ):
    """Copies meta-data to a circuit's meta-data
     where base can be either a binary_truth_table or a circuit"""
    settings = copy_metadata_settings()
    settings.copy_inputs = copy_inputs
    settings.copy_outputs = copy_outputs
    settings.copy_constants = copy_constants
    settings.copy_garbage = copy_garbage
    settings.copy_name = copy_name
    settings.copy_inputbuses = copy_inputbuses
    settings.copy_outputbuses = copy_outputbuses
    settings.copy_statesignals = copy_statesignals
    settings.copy_modules = copy_modules
    return py_copy_metadata( base, circ, settings )

def create_simulation_pattern( p, circ ):
    "Creates simulation pattern to be used with sequential_simulation from simulation file p according to circuit circ"
    return py_create_simulation_pattern( p, circ )

def control_lines( g ):
    "Returns a list of all control lines in a gate"
    return py_control_lines( g )

def target_lines( g ):
    "Returns a list of all target lines in a gate"
    return py_target_lines( g )

def expand_circuit( sub, circ ):
    "Expands a sub-circuit by the lines of its base circuit"
    return py_expand_circuit( sub, circ )

def extend_truth_table( spec ):
    "Removes the Don't Care Values of a binary truth table"
    return py_extend_truth_table( spec )

def fully_specified( spec ):
    "Checks whether a specification is fully specified"
    return py_fully_specified( spec )

def reverse_circuit( circ, dest = None ):
    """Reverses the circuit
    If dest is None, then the circuit is reversed in place, otherwise the reversed circuit is copied to dest"""
    if dest is None:
        return py_reverse_circuit( circ )
    else:
        return py_reverse_circuit( circ, dest )

def flatten_circuit( base, circ, keep_meta_data = False ):
    """Flattens the circuit base and writes the result to circ"""
    py_flatten_circuit( base, circ, keep_meta_data )

def find_non_empty_lines( circ_or_gate, begin = None, end = None ):
    """Returns the non empty lines in a circuit(range) or gate
    The first parameter can be a circuit or a gate. If the first paramater is a circuit,
    then the gates can be selected by a range from begin to end (exclusive)"""
    if begin is None or end is None:
        return py_find_non_empty_lines( circ_or_gate )
    else:
        return py_find_non_empty_lines( circ_or_gate, begin, end )

def find_empty_lines( circ_or_gate, begin_or_line_size = None, end = None ):
    """Returns the empty lines in a circuit(range) or gate
    The first parameter can be a circuit or a gate. If the first paramater is a circuit,
    then the gates can be selected by a range from begin (begin_or_line_size parameter) to end (exclusive).
    If the first parameter is a gate then the second parameter is used to specify the number of lines in the gate."""
    if not begin is None and not end is None:
        return py_find_non_empty_lines( circ_or_gate, begin_or_line_size, end )
    if not begin is None:
        return py_find_non_empty_lines( circ_or_gate, begin_or_line_size )
    else:
        return py_find_non_empty_lines( circ_or_gate )

### CORE/IO

def create_image( circ, generator = create_tikz_settings(), elem_width = 0.5, elem_height = 0.5, line_width = 0.3, control_radius = 0.1, target_radius = 0.2 ):
    """Creates a (LaTeX) image from a circuit and returns the generation code as string.

    The generator is an object which performs the generation of the image,
    the default generator is create_tikz_settings, which creates a TikZ image
    for the use in LaTeX. Another generator, which can be used is create_pstricks_settings.
    """
    generator.elem_width = elem_width
    generator.elem_height = elem_height
    generator.line_width = line_width
    generator.control_radius = control_radius
    generator.target_radius = target_radius
    return py_create_image( circ, generator )

def print_circuit( circ, print_inputs_and_outputs = False, print_gate_index = False, control_char = '*', line_char = '-', gate_spacing = 0, line_spacing = 0 ):
    "Prints the circuit in ASCII format"
    settings = print_circuit_settings()
    settings.print_inputs_and_outputs = print_inputs_and_outputs
    settings.print_gate_index = print_gate_index
    settings.control_char = control_char
    settings.line_char = line_char
    settings.gate_spacing = gate_spacing
    settings.line_spacing = line_spacing
    return py_print_circuit( circ, settings )

def print_statistics( circ, runtime = -1.0, main_template = "%1$sGates:            %2$d\nLines:            %3$d\nQuantum Costs:    %4$d\nTransistor Costs: %5$d\n", runtime_template = "Runtime:          %.2f\n" ):
    "Prints statistics about a circuit"
    settings = print_statistics_settings()
    settings.main_template = main_template
    settings.runtime_template = runtime_template
    return py_print_statistics( circ, runtime, settings )

def read_pattern( p, filename ):
    "Read-in routine for a simulation file in filename to p"
    return py_read_pattern( p, filename )

def read_pla( spec, filename, extend = True ):
    "Read-in routine for a binary truth table from a PLA file"
    settings = read_pla_settings()
    settings.extend = extend
    return py_read_pla( spec, filename, settings )

def read_realization( circ, filename ):
    "Read-in routine for a circuit from a RevLib realization file"
    return py_read_realization( circ, filename )

def read_specification( spec, filename ):
    "Read-in routine for a binary truth table from a RevLib specification file"
    return py_read_specification( spec, filename )

def write_blif( circ, filename, tmp_signal_name = "tmp", blif_mv = False, state_prefix = "out_", keep_constant_names = False ):
    "Writes a reversible circuit to a BLIF file"
    settings = write_blif_settings()
    settings.tmp_signal_name = tmp_signal_name
    settings.blif_mv = blif_mv
    settings.state_prefix = state_prefix
    settings.keep_constant_names = keep_constant_names
    return py_write_blif( circ, filename, settings )

def write_verilog( circ, filename, propagate_constants = True ):
    "Writes a reversible circuit to a Verilog file"
    settings = write_verilog_settings()
    settings.propagate_constants = propagate_constants
    return py_write_verilog( circ, filename, settings )

def write_realization( circ, filename, version = "2.0", header = "This file has been generated using RevKit %s (www.revkit.org)" % revkit_version() ):
    "Dumps a circuit as RevLib realization file"
    settings = write_realization_settings()
    settings.version = version
    settings.header = header
    return py_write_realization( circ, filename, settings )

def write_specification( spec, filename, version = "2.0", header = "This file has been generated using RevKit %s (www.revkit.org)" % revkit_version(), output_order = [] ):
    "Dumps a binary truth table as RevLib specification file"
    settings = write_specification_settings()
    settings.version = version
    settings.header = header
    settings.output_order = output_order
    return py_write_specification( spec, filename, settings )

### CORE/UTILS
def costs( circ, cost_function ):
    "Returns the costs for a circuit"
    return py_costs( circ, cost_function )

### ALGORITHMS

def bdd_synthesis( circ, filename, complemented_edges = True, reordering = 4, infofilename = "", dotfilename = "" ):
    "BDD Based Synthesis"
    settings = properties()
    settings.set_bool( "complemented_edges", complemented_edges )
    settings.set_unsigned( "reordering", reordering )
    settings.set_string( "infofilename", infofilename )
    settings.set_string( "dotfilename", dotfilename )

    statistics = properties()

    if py_bdd_synthesis( circ, filename, settings, statistics ):
        return dict( runtime = statistics.get_double( "runtime" ), node_count = statistics.get_unsigned( "node_count" ) )
    else:
        return error_msg( statistics )

def bdd_synthesis_func( complemented_edges = True, reordering = 4, infofilename = "", dotfilename = "" ):
    settings = properties()
    settings.set_bool( "complemented_edges", complemented_edges )
    settings.set_unsigned( "reordering", reordering )
    settings.set_string( "infofilename", infofilename )
    settings.set_string( "dotfilename", dotfilename )

    statistics = properties()

    return py_bdd_synthesis_func( settings, statistics )

def exact_synthesis ( circ, filename, solver = "MiniSAT", max_depth = 20, spec_incremental = False ):
    "Exact Synthesis using Boolean Satisfiability"
    settings = properties ()
    settings.set_string   ( "solver", solver)
    settings.set_unsigned ( "max_depth", max_depth )
    settings.set_bool     ( "spec_incremental", spec_incremental )

    statistics = properties()

    if (py_exact_synthesis ( circ, filename, settings, statistics ) ):
        return dict ( runtime = statistics.get_double ( "runtime" ) )
    else:
      return error_msg ( statistics )

def exact_synthesis_func ( solver = "MiniSAT", max_depth = 20, spec_incremental = False ):
    settings = properties()
    settings.set_string   ( "solver", solver)
    settings.set_unsigned ( "max_depth", max_depth )
    settings.set_bool     ( "spec_incremental", spec_incremental )

    statistics = properties()

    return py_exact_synthesis_func( settings, statistics )

def equivalence_check ( spec
    , impl
    , solver = "MiniSAT"
    , max_counterexample = 1
    , input_mapping = dict()
    , output_mapping = dict() ):
  "Formal (SAT) Equivalence Checker"
  settings = properties()
  settings.set_string   ( "solver", solver )
  settings.set_unsigned ( "max_counterexample", max_counterexample )

  settings.set_line_mapping ("input_mapping", input_mapping);
  settings.set_line_mapping ("output_mapping", output_mapping);

  statistics = properties ()

  if py_equivalence_check (spec, impl, settings, statistics):
    r = statistics.get_bool ("equivalent")
    if (r):
      return dict ( runtime = statistics.get_double ( "runtime" ), equivalent = True )
    else:
      return dict ( runtime = statistics.get_double ( "runtime"), equivalent = False, counterexample = statistics.get_counterexample ( "counterexample" )  )
  else:
    return error_msg ( statistics )


def embed_truth_table( spec, base, garbage_name = "g", output_order = [] ):
    "Embedding of an irreversible specification"
    settings = properties()
    settings.set_string( "garbage_name", garbage_name )
    settings.set_vector_unsigned( "output_order", output_order )

    statistics = properties()

    if py_embed_truth_table( spec, base, settings, statistics ):
        return dict( runtime = statistics.get_double( "runtime" ) )
    else:
        return error_msg( statistics )

def embed_truth_table_func( garbage_name = "g", output_order = [] ):
    settings = properties()
    settings.set_string( "garbage_name", garbage_name )
    settings.set_vector_unsigned( "output_order", output_order )

    statistics = properties()

    return py_embed_truth_table_func( settings, statistics )

def weighted_reordering( alpha = 0.5, beta = 0.5 ):
    return py_weighted_reordering( alpha, beta )

def esop_synthesis( circ, filename, separate_polarities = False, reordering = weighted_reordering(), garbage_name = "g" ):
    "ESOP Based Synthesis"
    settings = properties()
    settings.set_bool( "separate_polarities", separate_polarities )
    settings.set_cube_reordering_func( "reordering", reordering )
    settings.set_string( "garbage_name", garbage_name )

    statistics = properties()

    if py_esop_synthesis( circ, filename, settings, statistics ):
        return dict( runtime = statistics.get_double( "runtime" ) )
    else:
        return error_msg( statistics )

def esop_synthesis_func( separate_polarities = False, reordering = weighted_reordering(), garbage_name = "g" ):
    settings = properties()
    settings.set_bool( "separate_polarities", separate_polarities )
    settings.set_cube_reordering_func( "reordering", reordering )
    settings.set_string( "garbage_name", garbage_name )

    statistics = properties()

    return py_esop_synthesis_func( settings, statistics )

def kfdd_synthesis( circ, filename, default_decomposition = 2, reordering = 7, sift_factor = 2.5, sifting_growth_limit = 'a', sifting_method = 'v', dotfilename = "" ):
    "KFDD Based Synthesis"
    settings = properties()
    settings.set_unsigned( "default_decomposition", default_decomposition )
    settings.set_unsigned( "reordering", reordering )
    settings.set_double( "sift_factor", sift_factor )
    settings.set_char( "sifting_growth_limit", sifting_growth_limit )
    settings.set_char( "sifting_method", sifting_method )
    settings.set_string( "dotfilename", dotfilename )

    statistics = properties()

    if py_kfdd_synthesis( circ, filename, settings, statistics ):
        return dict( runtime = statistics.get_double( "runtime" ), node_count = statistics.get_unsigned( "node_count" ) )
    else:
        return error_msg( statistics )

def kfdd_synthesis_func( default_decomposition = 2, reordering = 0, sift_factor = 2.5, sifting_growth_limit = 'a', sifting_method = 'v', dotfilename = "" ):
    settings = properties()
    settings.set_unsigned( "default_decomposition", default_decomposition )
    settings.set_unsigned( "reordering", reordering )
    settings.set_double( "sift_factor", sift_factor )
    settings.set_char( "sifting_growth_limit", sifting_growth_limit )
    settings.set_char( "sifting_method", sifting_method )
    settings.set_string( "dotfilename", dotfilename )

    statistics = properties()

    return py_kfdd_synthesis_func( settings, statistics )

def quantum_decomposition( circ, base, helper_line_input = "w", helper_line_output = "w", gate_decomposition = standard_decomposition() ):
    "Quantum Decomposition of Reversible Circuits"
    settings = properties()
    settings.set_string( "helper_line_input", helper_line_input )
    settings.set_string( "helper_line_output", helper_line_output )
    settings.set_gate_decomposition_func( "gate_decomposition", gate_decomposition )

    statistics = properties()

    if py_quantum_decomposition( circ, base, settings, statistics ):
        return dict( runtime = statistics.get_double( "runtime" ) )
    else:
        return error_msg( statistics )

def quantum_decomposition_func( helper_line_input = "w", helper_line_output = "w", gate_decomposition = standard_decomposition() ):
    settings = properties()
    settings.set_string( "helper_line_input", helper_line_input )
    settings.set_string( "helper_line_output", helper_line_output )
    settings.set_gate_decomposition_func( "gate_decomposition", gate_decomposition )

    statistics = properties()

    return py_quantum_decomposition_func( settings, statistics )

def reed_muller_synthesis( circ, spec, bidirectional = True ):
    """Reed Muller Spectra Synthesis"""
    settings = properties()
    settings.set_bool( "bidirectional", bidirectional )
    statistics = properties()

    if py_reed_muller_synthesis( circ, spec, settings, statistics ):
        return dict( runtime = statistics.get_double( "runtime" ) )
    else:
        return error_msg( statistics )

def reed_muller_synthesis_func( bidirectional = True ):
    settings = properties()
    settings.set_bool( "bidirectional", bidirectional )

    statistics = properties()

    return py_reed_muller_synthesis_func( settings, statistics )

def transformation_based_synthesis( circ, spec, bidirectional = True ):
    "Transformation Based Synthesis"
    settings = properties()
    settings.set_bool( "bidirectional", bidirectional )

    statistics = properties()

    if py_transformation_based_synthesis( circ, spec, settings, statistics ):
        return dict( runtime = statistics.get_double( "runtime" ) )
    else:
        return error_msg( statistics )

def transformation_based_synthesis_func( bidirectional = True ):
    settings = properties()
    settings.set_bool( "bidirectional", bidirectional )

    statistics = properties()

    return py_transformation_based_synthesis_func( settings, statistics )

def transposition_based_synthesis( circ, spec ):
    "Simple Cycle Based Synthesis"
    settings = properties()
    statistics = properties()

    if py_transposition_based_synthesis( circ, spec, settings, statistics ):
        return dict( runtime = statistics.get_double( "runtime" ) )
    else:
        return error_msg( statistics )

def transposition_based_synthesis_func():
    settings = properties()
    statistics = properties()

    return py_transposition_based_synthesis_func( settings, statistics )

def swop( circ, spec, enable = True, exhaustive = False, synthesis = transformation_based_synthesis_func(), cf = gate_costs(), stepfunc = swop_step_func.empty() ):
    "SWOP - Synthesis With Output Permutation"
    settings = properties()
    settings.set_bool( "enable", enable )
    settings.set_bool( "exhaustive", exhaustive )
    settings.set_truth_table_synthesis_func( "synthesis", synthesis )
    settings.set_cost_function( "cost_function", cf )
    settings.set_swop_step_func( "stepfunc", stepfunc )

    statistics = properties()

    if py_swop( circ, spec, settings, statistics ):
        return dict( runtime = statistics.get_double( "runtime" ) )
    else:
        return error_msg( statistics )

def swop_func( enable = True, exhaustive = False, synthesis = transformation_based_synthesis_func(), cf = gate_costs(), stepfunc = swop_step_func.empty() ):
    settings = properties()
    settings.set_bool( "enable", enable )
    settings.set_bool( "exhaustive", exhaustive )
    settings.set_truth_table_synthesis_func( "synthesis", synthesis )
    settings.set_cost_function( "cost_function", cf )
    settings.set_swop_step_func( "stepfunc", stepfunc )

    statistics = properties()

    return py_swop_func( settings, statistics )

### Simulation
def simple_simulation( output, circ, inp, step_result = None ):
    "Very simple simulation, only efficient for small circuits"
    def inner_step_result( g, result ):
        # This function is only called when step_result is not None
        result_l = []
        for i in range( 0, len( inp ) ):
            result_l.append( result.test( i ) )
        step_result( g, result_l )

    del output[:]

    local_inp = bitset( len( inp ) )
    local_output = bitset()

    for i in range( 0, len( inp ) ):
        local_inp.set( i, inp[i] )

    settings = properties()
    if not step_result is None:
        settings.set_step_result_func( "step_result", step_result_func.from_callable( inner_step_result ) )

    statistics = properties()

    if py_simple_simulation( local_output, circ, local_inp, settings, statistics ):
        for i in range( 0, len( inp ) ):
            output.append( local_output.test( i ) )

        return dict( runtime = statistics.get_double( "runtime" ) )
    else:
        return error_msg( statistics )

def simple_simulation_func():
    settings = properties()

    statistics = properties()

    return py_simple_simulation_func( settings, statistics )

def partial_simulation( output, circ, inp, simulation = simple_simulation_func(), keep_full_output = False ):
    "Simulation considering constant inputs and garbage outputs"
    settings = properties()
    settings.set_simulation_func( "simulation", simulation )
    settings.set_bool( "keep_full_output", keep_full_output )

    del output[:]

    local_inp = bitset( len( inp ) )
    local_output = bitset()

    for i in range( 0, len( inp ) ):
        local_inp.set( i, inp[i] )

    statistics = properties()

    if py_partial_simulation( local_output, circ, local_inp, settings, statistics ):
        for i in range( 0, len( filter( lambda x: not x, circ.garbage ) ) ):
            output.append( local_output.test( i ) )

        return dict( runtime = statistics.get_double( "runtime" ) )
    else:
        return error_msg( statistics )

def partial_simulation_func( simulation = simple_simulation_func(), keep_full_output = False ):
    settings = properties()
    settings.set_simulation_func( "simulation", simulation )
    settings.set_bool( "keep_full_output", keep_full_output )

    statistics = properties()

    return py_simple_simulation_func( settings, statistics )

def sequential_simulation( outputs, circ, inputs, initial_state = dict(), vcd_filename = "", step_result = None ):
    "Sequential simulation considering stateful information"
    def inner_step_result( current_state, current_output ):
        # This function is only called when step_result is not None
        current_state_l = dict()
        for k in current_state:
            state_arr = []
            for i in range( current_state[k].size() ):
                state_arr.append( current_state[k].test( i ) )
            current_state_l[k] = state_arr

        current_output_l = []
        for i in range( current_output.size() ):
            current_output_l.append( current_output.test( i ) )
        l = step_result( current_state_l, current_output_l )
        new_input = bitset()
        if l is not None:
            new_input.clear()
            new_input.resize( len( l ), False )
            for i in range( len( l ) ):
                new_input.set( i, l[i] )
        return new_input

    settings = properties()
    settings.set_string( "vcd_filename", vcd_filename )
    settings.set_bitset_map( "initial_state", initial_state )
    if not step_result is None:
        settings.set_sequential_step_result_func( "step_result", sequential_step_result_func.from_callable( inner_step_result ) )

    del outputs[:]

    local_inputs = []
    for inp in inputs:
        bs = bitset( len( inp ) )
        for i in range( len( inp ) ):
            bs.set( i, inp[i] )
        local_inputs.append( bs )

    local_outputs = []

    statistics = properties()

    if py_sequential_simulation( local_outputs, circ, local_inputs, settings, statistics ):
        for lout in local_outputs:
            output = []
            for i in range( lout.size() ):
                output.append( lout.test( i ) )
            outputs.append( output )

        return dict( runtime = statistics.get_double( "runtime" ) )
    else:
        return error_msg( statistics )

### Optimization
def adding_lines( circ, base, additional_lines = 1 ):
    "Adding Lines Optimization"
    settings = properties()
    settings.set_unsigned( "additional_lines", additional_lines )

    statistics = properties()

    if py_adding_lines( circ, base, settings, statistics ):
        return dict( runtime = statistics.get_double( "runtime" ) )
    else:
        return error_msg( statistics )

def adding_lines_func( additional_lines = 1 ):
    settings = properties()
    settings.set_unsigned( "additional_lines", additional_lines )

    statistics = properties()

    return py_adding_lines_func( settings, statistics )

def embed_and_synthesize( embedding = embed_truth_table_func(), synthesis = transformation_based_synthesis_func(), timeout = 0 ):
    return py_embed_and_synthesize( embedding, synthesis, timeout )

def line_reduction( circ, base, max_window_lines = 6, max_grow_up_window_lines = 9, window_variables_threshold = 17, simulation = simple_simulation_func(), window_synthesis = embed_and_synthesize() ):
    "Line Reduction Optimization"
    settings = properties()
    settings.set_unsigned( "max_window_lines", max_window_lines )
    settings.set_unsigned( "max_grow_up_window_lines", max_grow_up_window_lines )
    settings.set_unsigned( "window_variables_threshold", window_variables_threshold )
    settings.set_simulation_func( "simulation", simulation )
    settings.set_window_synthesis_func( "window_synthesis", window_synthesis )

    statistics = properties()

    if py_line_reduction( circ, base, settings, statistics ):
        return dict( runtime = statistics.get_double( "runtime" ), \
                     num_considered_windows = statistics.get_unsigned( "num_considered_windows" ), \
                     skipped_max_window_lines = statistics.get_unsigned( "skipped_max_window_lines" ), \
                     skipped_ambiguous_line = statistics.get_unsigned( "skipped_ambiguous_line" ), \
                     skipped_no_constant_line = statistics.get_unsigned( "skipped_no_constant_line" ), \
                     skipped_synthesis_failed = statistics.get_unsigned( "skipped_synthesis_failed" ) )
    else:
        return error_msg( statistics )

def line_reduction_func( max_window_lines = 6, max_grow_up_window_lines = 9, window_variables_threshold = 17, simulation = simple_simulation_func(), window_synthesis = embed_and_synthesize() ):
    settings = properties()
    settings.set_unsigned( "max_window_lines", max_window_lines )
    settings.set_unsigned( "max_grow_up_window_lines", max_grow_up_window_lines )
    settings.set_unsigned( "window_variables_threshold", window_variables_threshold )
    settings.set_simulation_func( "simulation", simulation )
    settings.set_window_synthesis_func( "window_synthesis", window_synthesis )

    statistics = properties()

    return py_line_reduction_func( settings, statistics )

def lnn_optimization(circ, base, reordering = 0 ):
    "LNN Optimization"
    settings = properties()
    settings.set_unsigned( "reordering", reordering )

    statistics = properties()

    if py_lnn_optimization(circ, base, settings, statistics):
        return dict(runtime = statistics.get_double( "runtime") )
    else:
        return error_msg( statistics)

def lnn_optimization_func( reordering = 0 ):
    settings = properties()
    settings.set_unsigned( "reordering", reordering )

    statistics = properties()

    return py_lnn_optimization_func( settings, statistics)

def shift_window_selection_func( window_length = 10, offset = 1 ):
    return py_shift_window_selection_func( window_length, offset )

def line_window_selection_func():
    return py_line_window_selection_func()

def resynthesis_optimization_func( synthesis = transformation_based_synthesis_func(), simulation = simple_simulation_func() ):
    return py_resynthesis_optimization_func( synthesis, simulation )

def window_optimization( circ, base, select_window = shift_window_selection_func(), optimization = resynthesis_optimization_func(), cf = gate_costs() ):
    "Window Optimization"
    settings = properties()
    settings.set_select_window_func( "select_window", select_window )
    settings.set_optimization_func( "optimization", optimization )
    settings.set_cost_function( "cost_function", cf )

    statistics = properties()

    if py_window_optimization( circ, base, settings, statistics ):
        return dict( runtime = statistics.get_double( "runtime" ) )
    else:
        return error_msg( statistics )

def window_optimization_func( select_window = shift_window_selection_func(), optimization = resynthesis_optimization_func(), cf = gate_costs() ):
    settings = properties()
    settings.set_select_window_func( "select_window", select_window )
    settings.set_optimization_func( "optimization", optimization )
    settings.set_cost_function( "cost_function", cf )

    statistics = properties()

    return py_window_optimization_func( settings, statistics )



### GUI
def init_gui():
    from PyQt4 import QtGui
    return QtGui.QApplication([])

def display_circuit( circ ):
    import revkitui

    w = revkitui.CircuitView( circ )
    w.setWindowTitle( "Circuit View" )
    w.show()
    return w
    #a.exec_()


### CORE/FUNCTIONS

def circuit_to_truth_table( circ, spec, simulation = simple_simulation_func() ):
    """Writes a binary truth table to spec
     by fully simulating circ using the simulation function given"""
    return py_circuit_to_truth_table( circ, spec, simulation )
