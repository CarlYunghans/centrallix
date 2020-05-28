#include <stdio.h>
#include <assert.h>
#include <string.h>
#include "b+tree.h"
#include "newmalloc.h"
long long
test(char** tname)
   	{
	printf("\n");
	int i;
    	int iter;
	int tmp;

	*tname = "b+tree-03 5 Keys in root";
	iter = 8000000;
	
	pBPTree root = bptNew();
	//printf("A\n");
	
	/*void* copy;
        copy = nmSysMalloc(5);
        if (!copy) 
		return -1; 
	
	memcpy(copy, "Tommy", 5);
          */      
        root->Keys[0].Length = 5;
        root->Keys[0].Value = "Tommy\0"; 
	root->nKeys++;	
	root->Keys[1].Length = 26;
	root->Keys[1].Value = "abcdefghijklmnopqrstuvwxyz\0";
	root->nKeys++;
	root->Keys[2].Length = 1;
	root->Keys[2].Value = "a\0";
	root->nKeys++;
	root->Keys[3].Length = 10;
	root->Keys[3].Value = "0123456789\0";
	root->nKeys++;
	root->Keys[4].Length = 2;
	root->Keys[4].Value = "!?\0";
	root->nKeys++;

	printf("%d\n", root->nKeys);
	
	//strcpy(root->Keys[0].Value, "Tommy");
	//printf("B\n");
	tmp = 0;
	bpt_PrintTree(root);
	for(i=0;i<iter;i++)
	 	{
		//tmp = bpt_PrintTree(NULL);
		assert (tmp == 0);
		}

	printf("\n");
	
    	return iter*4;
    	}
