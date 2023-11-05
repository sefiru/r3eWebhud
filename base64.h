#pragma once
#ifndef HEX_TO_BASE64_H
#define HEX_TO_BASE64_H
/*
  Function to convert a hexadecimal-encoded string to a base64-encoded string.
  params : hexadecimal-encoded string
  returns : string - or a null pointer if allocation failed

  The return value must be deallocated using free().
*/
char* hex_to_base64(const char* hex_string);
#endif