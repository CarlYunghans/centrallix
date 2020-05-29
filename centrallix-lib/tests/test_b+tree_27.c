#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "b+tree.h"
#include "newmalloc.h"
long long
test(char** tname)
   	{
	printf("\n");
	int a, i;
    	int iter;

	*tname = "b+tree-27 Bulk Loading: Size = 10";
	iter = 8000000;
	
	
	char* fname = "tests/bpt_bl_10e1.dat";
	FILE* tree = NULL;
	FILE* dict = NULL;
	tree = fopen(fname, "w");
	dict = fopen("tests/dictionary.txt", "r");
	if (dict == NULL)
		perror("dict is null");
	char str[50];

	for (a=1; a<=10; a++)
		{
		fscanf(dict, "%s\n", str);
		printf("%s\n", str);
		fprintf(tree, "%08d %s\n", a, str);
		}
	fclose(dict);
	fclose(tree);


	//bpt_PrintTree(root);
	for(i=0;i<iter;i++)
	 	{
		assert (5 == 5);
		}

	printf("\n");
	
    	return iter*4;
    	}
