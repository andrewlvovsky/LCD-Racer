#include <stdio.h>

/**
  * Convert a number to a C style string.
  * Inputs:
  *     num:    The number to be converted
  *     string: An allocated array that is the same size as the number of
  *             digits in the number + 1
  *     size:   The length of the allocated char array.
  * Return: The array terminated with a null character.
  */
unsigned char * LCD_To_String(unsigned short num, unsigned char * string, unsigned char size) {

    string[size - 1] = '\0';

    for (short i = size - 2; i >= 0; i--) {
        string[i] = num % 10 + '0';
        num /= 10;
    }
    return string;
}
