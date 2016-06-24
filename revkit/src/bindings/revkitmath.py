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

# -*- coding: utf-8 -*
"""RevKit Math module for matrix manipulation"""
import numpy
from numpy import array, dot, conj, transpose, eye, log2, arange, kron
from revkit import gate_type

# Matrices
def create_not():
    """Creates a NOT Matrix"""
    return array( [[0, 1], [1, 0]], dtype = complex )

def create_cnot():
    """Creates a CNOT Matrix"""
    return create_matrix( create_not(), [0], [1], 2 )

def create_toffoli( controls = [0, 1], target = 2, lines = 3 ):
    """Creates a Toffoli Matrix"""
    return create_matrix( create_not(), controls, [target], lines )

def create_swap():
    """Creates a SWAP Matrix"""
    return array( [[1, 0, 0, 0], [0, 0, 1, 0], [0, 1, 0, 0], [0, 0, 0, 1]],
                  dtype = complex )

def create_fredkin( controls = [0], targets = [1, 2], lines = 3 ):
    """Creates a Fredkin Matrix"""
    return create_matrix( create_swap(), controls, targets, lines )

def create_v( controls = [0], target = 1, lines = 2 ):
    """Creates a V Matrix"""
    return create_matrix( array( [[1+1j, 1-1j], [1-1j, 1+1j]] ) / 2,
                          controls, [target], lines )

def create_vplus( controls = [0], target = 1, lines = 2 ):
    """Creates a V+ Matrix"""
    return create_matrix( array( [[1-1j, 1+1j], [1+1j, 1-1j]] ) / 2,
                          controls, [target], lines )

def prepend_empty_lines( mat, lines = 1 ):
    """Prepends 'lines' empty lines to the matrix"""
    return kron( eye( 2**lines ), mat )

def append_empty_lines( mat, lines = 1 ):
    """Appends 'lines' empty lines to the matrix"""
    return kron( mat, eye( 2**lines ) )

def prepend_controls( mat, num ):
    """Prepends 'lines' control lines to the matrix"""
    for _ in arange( num ):
        lines = mat.shape[0]
        unit = eye( lines * 2, dtype = complex )
        unit[lines:lines*2, lines:lines*2] = mat
        mat = unit
    return mat

def create_matrix( mat, controls, targets, lines ):
    """Creates a Matrix from a base matrix 'mat'"""
    if check_unitary( mat ) == False:
        raise Exception( "expected unitary matrix" )

    if int( log2( mat.shape[0] ) ) != len( targets ):
        raise Exception( "size of matrix and number "
                         "of target lines do not match" )

    if len( set( controls ).intersection( set( targets ) ) ) != 0:
        raise Exception( "target line collides with control lines" )

    mat = prepend_empty_lines( prepend_controls( mat, len( controls ) ),
                            lines - len( controls ) - len( targets ) )

    control_positions = [ lines - len( targets ) - len( controls ) + i
                         for i in arange( len( controls ) ) ]

    for i in arange( len( targets ) ):
        current = targets[i]
        new = lines - len( targets ) + i
        mat = swaplines( mat, current, new )

        if current in control_positions:
            control_positions.remove( current )
            control_positions.append( new )

    for i in arange( len( controls ) ):
        current = control_positions.pop()
        new = controls[i]
        mat = swaplines( mat, current, new )

    return mat

# Vectors
def create_vector( value, lines ):
    """Creates a vector for value"""
    vec = [0] * 2**lines
    vec[value] = 1
    return vec

# Operations
def circuit_to_matrix( circ ):
    """Creates a matrix for a circuit"""
    mat = eye( 2**circ.lines )
    for gate in circ:
        targets = list( gate.targets )

        if gate.type == gate_type.toffoli:
            gatemat = create_toffoli( list( gate.controls ), targets[0],
                                      circ.lines )
        elif gate.type == gate_type.fredkin:
            gatemat = create_fredkin( list( gate.controls ), targets,
                                      circ.lines )
        elif gate.type == gate_type.v:
            gatemat = create_v( list( gate.controls ), targets[0],
                                circ.lines )
        elif gate.type == gate_type.vplus:
            gatemat = create_vplus( list( gate.controls ), targets[0],
                                    circ.lines )
        else:
            raise Exception( "gate type is not supported" )

        mat = dot( gatemat, mat )
    return mat

def nat_evaluate( mat, value, lines ):
    """Evalate a matrix using a natural number and return natural"""
    return [i for (i, v) in enumerate( dot( mat,
                                           create_vector( value, lines ) ) )
            if v == 1][0]

def bin_evaluate( mat, value ):
    """Evalate a matrix using a binary number and return natural"""
    return nat_evaluate( mat, sum( [int(c) * 2**i for (i, c)
                               in enumerate(reversed(value))] ), len( value ) )

def _swaplines( first = 0, second = 1, lines = 2 ):
    """Recursive function for _swaplines"""
    if first == second:
        return eye( 2**lines, dtype = complex )

    if first > second:
        first ^= second
        second ^= first
        first ^= second

    if first + 1 == second:
        return append_empty_lines( prepend_empty_lines(
            create_swap(), first ), lines - second - 1 )
    else:
        mat = _swaplines( second - 1, second, lines )
        mat = dot( mat, _swaplines( first, second - 1, lines ) )
        return dot( mat, _swaplines( second - 1, second, lines ) )

def swaplines( mat, first, second ):
    """Swap two lines in a matrix"""
    swapmat = _swaplines( first, second, int( log2( mat.shape[0] ) ) )
    return dot( swapmat, dot( mat, swapmat ) )

def check_unitary( mat ):
    """Checks a matrix for being unitary"""
    if mat.ndim != 2:
        return False
    if mat.shape[0] != mat.shape[1]:
        return False
    if numpy.any( dot( conj( transpose( mat ) ), mat ) != eye( mat.shape[0] ) ):
        return False
    return True
