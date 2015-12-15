#ifndef CONSTANTS_H
#define CONSTANTS_H

// TODO: no sería mejor utilizar macros para estas constantes? Para que el
// lector del código tenga un poco más de inf. sobre donde ir a buscar la
// definición de estos valores...

// Initial values of attributes.
const int integer_initial_value = 0;
const float float_initial_value = 0.0;
const bool boolean_initial_value = true;

// Type's width (in bytes).
const unsigned int integer_width = 4;
const unsigned int float_width = 4;
const unsigned int boolean_width = 4;
const unsigned int reference_width = 4;

// Quantity of integer parameters that are passed into registers.
#define INT_PARAMS_INTO_REG 6
#define RBP_REGISTER_SIZE 8

#endif
