#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <ctype.h>
#include <stddef.h>
#include "ps_server.h"

/*
 * Function: str_match
 * Parameters:
 *     str1: first string to be compared
 *     str2: second string to be compared
 * Returns: -1 if str1 less than str2
 *           0 if str1 equals str2
 *           1 if str1 greater than str2
 * Description: compare str1 and str2, only compares
 *		alphanumeric characters. If non-alphanumeric
 *		character is found it is ignored and next
 *		character is checked. Case insensitive.
 */

int str_match(char *str1, char *str2)
{
	while (*str1 != '\0' && *str2 != '\0') {

		if (isalnum(*str1) == 0 && isalnum(*str2) == 0) {
			str1++;
			str2++;
		} else if (isalnum(*str1) != 0 && isalnum(*str2) == 0) {
			str2++;
		} else if (isalnum(*str2) != 0 && isalnum(*str1) == 0) {
			str1++;
		} else {
			if (*str1 < *str2) {
				if (*str1 >= 'A' && *str1 <= 'Z') {
					if (*str2 >= 'A' && *str2 <= 'Z')
						return -1;
					if (*str1+32 > *str2)
						return 1;
					if (*str1+32 == *str2) {
						str1++;
						str2++;
						continue;
					}
				}
				return -1;
			}
			if (*str1 > *str2) {
				if (*str2 >= 'A' && *str2 <= 'Z') {
					if (*str1 >= 'A' && *str1 <= 'Z')
						return 1;
					if (*str2+32 > *str1)
						return -1;
					if (*str2+32 == *str1) {
						str1++;
						str2++;
						continue;
					}
				}
				return 1;
			}
			str1++;
			str2++;

		}
	}
	if (*str1 == '\0' && *str2 != '\0')
		return -1;
	if (*str1 != '\0' && *str2 == '\0')
		return 1;

	return 0;
}
