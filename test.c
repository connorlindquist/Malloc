#include "my_malloc.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>
//#include "list.h"

int main() {
	/* ----------------- mymemmove() ---------------------- */
	int ints_a[10] = {0,1,2,3,4,5,6,7,8,9};
	int* a = ints_a;
	int ints_b[10] = {9,8,7,6,5,4,3,2,1,0};
	int* b = ints_b;

	my_memmove(b, a, sizeof(int) * 10);
	int pass = 1;
	for(int i = 0; i < 10; i++)
	{
		if(ints_a[i] != ints_b[i])
		{
			pass = 0;
		}
	}
	printf("%d my_memmove() copy backwards\n", pass);

	int* c = a + 5;
	int ints_c[10] ={5,6,7,8,9,5,6,7,8,9};
	pass = 1;
	my_memmove(a, c, sizeof(int) * 5);
	for(int i = 0; i < 10; i++)
	{
		if(ints_a[i] != ints_c[i])
		{
			pass = 0;
		}
	}
	printf("%d my_memmove() copy forwards\n", pass);

	/* ---------------- my_malloc() ---------------- */
	// TEST 1 simple malloc test
	int* ptr_1 = my_malloc(sizeof(int));
	*ptr_1 = 150;
	printf("%d first malloc call works\n", *ptr_1 == 150);

	char* ptr_2 = my_malloc(sizeof(char) * 25);
	char arr[25] = {"1 my_malloc is working!\n"};
	for(int i = 0; i < 25; i++)
	{
		*(ptr_2+i) = arr[i];
	}
	printf("%s", ptr_2);

	// TEST 2 malloc errors
	int* ptr_4 = my_malloc(2049-sizeof(metadata_t));
	printf("%d my_malloc returns NULL when request size exceeds 2048\n", ptr_4 == NULL);

	/* =================== my_free() ================= */
	my_free(ptr_1);
	my_free(ptr_2);
	for(int i = 0; i < 6; i++)
	{
		int* test_ptr = my_malloc(i * 300 + 1);
		printf("%d my_free() is working!\n", test_ptr != NULL);
		my_free(test_ptr);

	}


	/* ================== my_calloc() ================= */
	int* int_ptr = my_calloc(10, sizeof(int));
	printf("using my_calloc of size 10 to allocate an int ptr....\n[");
	for(int i = 0; i < 9; i++)
	{
		printf("%d, ", *(int_ptr + i));
	}
	printf("%d]\n",*(int_ptr + 9));
	my_free(int_ptr);

	/* ================= extensive tests =============== */

	/*for(int i = 0; i < 2000; i++)
	{
		int* test_ptr = my_malloc(i + 4);
		*test_ptr = i;
		my_free(test_ptr);
	}*/

	return 0;

}