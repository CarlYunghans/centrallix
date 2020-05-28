#ifdef HAVE_CONFIG_H
#include "cxlibconfig-internal.h"
#endif
#include <stdio.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include "b+tree.h"
#include "newmalloc.h"

/************************************************************************/
/* Centrallix Application Server System 				*/
/* Centrallix Base Library						*/
/* 									*/
/* Copyright (C) 1998-2019 LightSys Technology Services, Inc.		*/
/* 									*/
/* You may use these files and this library under the terms of the	*/
/* GNU Lesser General Public License, Version 2.1, contained in the	*/
/* included file "COPYING".						*/
/* 									*/
/* Module: 	b+tree.c, b+tree.h  					*/
/* Author:	Greg Beeley (GRB)					*/
/* Creation:	September 11, 2019					*/
/* Description:	B+ Tree implementation.					*/
/************************************************************************/
pBPTree queue = NULL;

int bpt_i_Fake()
{
	return 0;
}

/*** bptNew() - allocate and initialize a new B+ Tree
 ***/
pBPTree
bptNew()
    {
	pBPTree this;

	/** Allocate **/
	this = (pBPTree)nmMalloc(sizeof(BPTree));
	if (!this)
	    return NULL;

	/** Init **/
	if (bptInit(this) != 0)
	    {
	    nmFree(this, sizeof(BPTree));
	    return NULL;
	    }

    return this;
    }


/*** bptInit() - initialize an already-allocated B+ Tree
 ***/
int
bptInit(pBPTree this)
    {
	
	/** Should not be passed NULL **/
	assert (this != NULL);

	/** Clear out the data structure **/
	this->Parent = this->Next = this->Prev = NULL;
	this->nKeys = 0;
	this->IsLeaf = 1;

    return 0;
    }


/*** bptFree() - deinit and deallocate a B+ Tree
 ***/
void
bptFree(pBPTree this)
    {
	bptDeInit(this);
	nmFree(this, sizeof(BPTree));

    return;
    }


/*** bptDeInit() - deinit a B+ Tree, but don't deallocate
 ***/
int
bptDeInit(pBPTree this)
    {
	
	/** Should not be passed NULL **/
	assert (this != NULL);

    int i;

	/** Deallocate children **/
	if (!this->IsLeaf)
	    {
	    for(i=0; i<this->nKeys+1; i++)
			{
			bptFree(this->Children[i].Child);
			}
	    }

	/** Deallocate key values **/
	for(i=0; i<this->nKeys; i++)
	    {
	    nmSysFree(this->Keys[i].Value);
	    }
	this->nKeys = 0;

	this->Parent = this->Next = this->Prev = NULL;

    return 0;
    }


/*** bpt_i_Compare() - compares two key values.  Return value is greater
 *** than 0 if key1 > key2, less than zero if key1 < key2, and equal to
 *** zero if key1 == key2.
 ***/
int
bpt_i_Compare(char* key1, int key1_len, char* key2, int key2_len)
    {
    int len, rval;

	/** Common length **/
	if (key1_len > key2_len)
	    len = key2_len;
	else
	    len = key1_len;

	/** Compare **/
	rval = memcmp(key1, key2, len);

	/** Initial parts same: compare based on lengths of keys. **/
	if (rval == 0)
	    rval = key1_len - key2_len;

    return rval;
    }


/*** bpt_i_Split() - split a node. 
 *** Returns the new right node or NULL if the split location is invalid
 *** or the new node cannot be created.
 *** A new node is created and linked to the right of the input node.
 *** After the split, the input node contains [0, split_loc) and the new
 *** right node contains [split_loc, nkeys). A split location input of
 *** nKeys causes an empty node to be created and linked.
 ***/
pBPTree bpt_i_Split(pBPTree node, int split_loc)
    {
    pBPTree right_node = NULL;

    if(node == NULL
    || split_loc < 0
    || split_loc > node->nKeys)
        {
        return NULL;
        }

    right_node = bptNew();

    if(!right_node)
        {
        return NULL;
        }

    memmove(right_node->Keys, &node->Keys[ split_loc ], sizeof(BPTreeKey) * (node->nKeys - split_loc));

    /** For index nodes, the right node left child is set elsewhere, so children are offset by 1 **/
    memmove(&right_node->Children[ 1 ], &node->Children[ split_loc + 1 ], sizeof(BPTreeVal) * (node->nKeys - split_loc));

    right_node->nKeys = node->nKeys - split_loc;
    node->nKeys = split_loc;

    right_node->IsLeaf = node->IsLeaf;
    if(!right_node->IsLeaf)
        {
        for(int i = 1; i <= right_node->nKeys; i++)
            {
            right_node->Children[ i ].Child->Parent = right_node;
            }
        }

    /** Link the node to the right of the current node **/
    right_node->Next = node->Next;
    if(right_node->Next != NULL)
        {
        right_node->Next->Prev = right_node;
        }
    node->Next = right_node;
    right_node->Prev = node;

    return right_node;
    }


/*** bpt_i_Push() - push a value to the left or right sibling
 ***/
int
bpt_i_Push(pBPTree this)
    {
	return 0;
    }

/*** bpt_i_get_left() - finds the index of the parent's pointer
     that is for the node to the left of the key to be intserted
***/
int bpt_i_get_left(BPTree * parent, BPTree *l){

    int dx = -1;
    while(dx <= parent->nKeys && parent->Children[dx].Child != l){
        dx++;
    }
    return dx;


}

/*** bpt_i_Scan() - scan a node's keys looking for a key
 ***/
int
bpt_i_Scan(pBPTree this, char* key, int key_len, int *locate_index)
    {
    int i, rval;

	/** Find it **/
	for(i=0; i<this->nKeys; i++)
	    {
	    rval = bpt_i_Compare(key, key_len, this->Keys[i].Value, this->Keys[i].Length);
	    if (rval <= 0)
		break;
	    }

	*locate_index = i;

    return rval;
    }

/*** bpt_i_Find() - locate a key/value pair in the tree.  If not
 *** found, determine the location where it should be inserted.  Returns
 *** 0 if found, -1 if not found.
 ***/
int
bpt_i_Find(pBPTree this, char* key, int key_len, pBPTree *locate, int *locate_index)
    {
    int rval;

	/** Scan this node **/
	rval = bpt_i_Scan(this, key, key_len, locate_index);
	if (this->IsLeaf)
	    {
	    *locate = this;
	    return rval;
	    }

	/** Scan the selected child node **/
	rval = bpt_i_Find(this->Children[*locate_index].Child, key, key_len, locate, locate_index);

    return rval;
}


/*** bpt_i_Insert() - insert a key/value pair into a node. Does NOT check
 *** to see if there is room - the caller must do that first.
 *** Returns 0 if successful and an error code otherwise.
 ***
 *** NOTE: This insert uses the convention that the value pointer of a leaf
 *** key is at node->Children[ key_index + 1 ], since this simplifies the insert
 *** the insert and seems consistent with how the last layer of index nodes are
 *** treated (in that the leaf child of an index key that contains the index
 *** key is at Children[ key_index + 1 ])
 ***/
int
bpt_i_Insert(pBPTree this, char* key, int key_len, void* data, int idx)
    {
    void* copy;

    if(this == NULL
    || key == NULL
    || key_len <= 0 
    || data == NULL
    || idx < 0
    || idx > this->nKeys)
        {
        return -1;
        }

	/** Make a copy of the key **/
	copy = nmSysMalloc(key_len);
	if (!copy)
	    return -1;
	memcpy(copy, key, key_len);

	/** Make room for the key and value **/
	if (idx != this->nKeys)
	    {
	    memmove(this->Keys+(idx + 1), this->Keys+idx, sizeof(BPTreeKey) * (this->nKeys - idx));
	    memmove(this->Children + (idx + 1) +1, this->Children + idx + 1, sizeof(BPTreeVal) * (this->nKeys - idx));
	    }

    /** Set it **/
    this->nKeys++;
    this->Keys[ idx ].Length = key_len;
    this->Keys[ idx ].Value = copy;
    if(this->IsLeaf)
        {
        this->Children[ idx + 1 ].Ref = data;
        }
    else
        {
        this->Children[ idx + 1 ].Child = data;
        this->Children[ idx + 1 ].Child->Parent = this;
        }

    return 0;
    }

void
bpt_i_Enqueue(pBPTree this)//global var or does this method work
        {
        pBPTree curr;
        if (queue == NULL)
                {
                queue = this;
                queue->Next = NULL;
		}
        else
                {
                curr = queue;
                while (curr->Next != NULL)
                        curr = curr->Next;
                curr->Next = this;
                this->Next = NULL;
                }
	return;
        }

pBPTree
bpt_i_Dequeue()
        {
        pBPTree head = queue;
        queue = queue->Next;
	head->Next = NULL;
        return head;
        }

int
bpt_i_height(pBPTree root)
        {
        int h = 0;
        pBPTree curr = root;
        while (!curr->IsLeaf)
                {
                curr = curr->Children[0].Child;
                h++;
                }
        return h;
        }

int
bpt_i_PathToRoot(pBPTree this, pBPTree root)
        {
        int len = 0;
        pBPTree curr = this;
        while (curr != root)
                {
                curr = curr->Parent;
                len++;
                }
        return len;
        }



/*** bptAdd() - add a key/value pair to the tree.  Returns 1 if the
 *** key/value pair already exists, 0 on success, or -1 on error.
 ***/
int
bptAdd(pBPTree this, char* key, int key_len, void* data)
{
    pBPTree node, new_node = NULL;
    int dx;

	/** See if it is there. **/

	if (bpt_i_Find(this, key, key_len, &node, &dx) == 0)
	    {
	    /** Already exists.  Don't add. **/
	    return 1;
	    }

	/** Not enough room? **/
	if (node->nKeys == BPT_SLOTS)
	    {
	    new_node = bpt_i_Split(node, CEIL_HALF_OF_LEAF_SLOTS);

	    /** Error condition if new_node is NULL. **/
	    if (!new_node)
		return -1;
	    }

	/** Which node are we adding to? **/
	if (new_node){
    	if (dx > BPT_SLOTS / 2){
			node = new_node;
			dx -= (BPT_SLOTS / 2);
		}
	}

	/** Insert the item **/
	if (bpt_i_Insert(node, key, key_len, data, dx) < 0){
		return -1;
	}

    return 0;
    }


/*** bptLookup() - find a value for a given key.  Returns NULL if the
 *** key does not exist in the tree
 ***/
void*
bptLookup(pBPTree this, char* key, int key_len)
    {
    pBPTree node;
    int idx;

	/** Look it up **/
	if (bpt_i_Find(this, key, key_len, &node, &idx) == 0)
	    {
	    /** Found **/
	    return node->Children[idx].Ref;
	    }

    return NULL;
    }


/*** bptRemove() - removes a key/value pair from the tree.  Returns -1
 *** if the key was not found.
 ***/
int
bptRemove(pBPTree this, char* key, int key_len)
    {
	return -1;
    }


/*** bptClear() - clears the tree of all key/value pairs.  Calls the provided
 *** free_fn for each key/value pair, as:  free_fn(free_arg, value)
 ***/
int
bptClear(pBPTree this, int (*free_fn)(), void* free_arg)
    {
    int i;

	/** Clear child subtrees first **/
	for(i=0; i<this->nKeys; i++)
	    {
	    if (this->IsLeaf)
		{
		free_fn(free_arg, this->Children[i].Ref);
		}
	    else
		{
		bptClear(this->Children[i].Child, free_fn, free_arg);
		nmFree(this->Children[i].Child, sizeof(BPTree));
		}
	    }
	if (!this->IsLeaf)
	    {
	    bptClear(this->Children[this->nKeys].Child, free_fn, free_arg);
	    nmFree(this->Children[this->nKeys].Child, sizeof(BPTree));
	    }

	/** Clear key/value pairs **/
	for(i=0; i<this->nKeys; i++)
	    {
	    nmSysFree(this->Keys[i].Value);
	    free_fn(free_arg, this->Keys[i].Value);
	    }

	/** Reset the root node **/
	if (!this->Parent)
	    bptInit(this);

    return 0;
    }

int
bpt_PrintTree(pBPTree root)
        {
        pBPTree curr = NULL;
        int i = 0;
        int rank = 0;
        int new_rank = 0;

        if (root == NULL)
                {
                printf("Empty tree\n");
                return 1;
                }

        bpt_i_Enqueue(root);
        while (queue != NULL)
                {
                curr = bpt_i_Dequeue();
                if (curr->Parent != NULL && curr == curr->Parent->Children[0].Child)
                        {
                        new_rank = bpt_i_PathToRoot(curr, root);
                        if (new_rank != rank)
                                {
                                rank = new_rank;
                                printf("\n");
                                }
                        }
                for (i=0; i<curr->nKeys; i++)
                        printf("%s ", curr->Keys[i].Value);
                if (!curr->IsLeaf)
                        for (i=0; i<=curr->nKeys; i++)
                                bpt_i_Enqueue(curr->Children[i].Child);
                printf("| ");
                }
        printf("\n");
	return 0;
    }