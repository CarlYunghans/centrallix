#ifdef HAVE_CONFIG_H
#include "cxlibconfig-internal.h"
#endif
#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <fcntl.h>
#include <string.h>
#include <assert.h>
#include <math.h>
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
	memset(this, 0, sizeof(BPTree));
	/** Clear out the data structure **/
	this->Parent = this->Next = this->Prev = NULL;
	this->nKeys = 0;
	this->IsLeaf = 1;

    return 0;
    }


/*** bptFree() - deinit and deallocate a B+ Tree
 ***/

int
bptInitRoot(BPTreeRoot *this){

	pBPTree root = bptNew();
	root->Parent = root->Next = root->Prev = NULL;
	root->nKeys = 0;
	root->IsLeaf = 1;
	this->root = root;
	return 0;
}

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

   	int i;
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
        for(i = 1; i <= right_node->nKeys; i++)
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
	
	if (this == NULL)
		return -1;
	/** Scan this node **/
	rval = bpt_i_Scan(this, key, key_len, locate_index);
	if (this->IsLeaf)
	    	{
		//printf("LEAF\n");
		*locate = this;
	    	if (rval != 0)
			rval = -1;
		return rval;
	    	}
	/** Scan the selected child node **/
	else
		rval = bpt_i_Find(this->Children[*locate_index].Child, key, key_len, locate, locate_index);
	//printf("RETURNING\n");
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
bpt_i_Insert(pBPTree this, char* key, int key_len, pBPTreeVal data, int idx)
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

	/** Make a copy of the key**/ 
	copy = nmSysMalloc(key_len);
	if (!copy)
	    return -1;
	memcpy(copy, key, key_len);

	/** Make room for the key and value**/
	if (idx != this->nKeys)
	    {
	    memmove(this->Keys+(idx + 1), this->Keys+idx, sizeof(BPTreeKey) * (this->nKeys - idx));
	    memmove(this->Children + (idx + 1) +1, this->Children + idx + 1, sizeof(BPTreeVal) * (this->nKeys - idx));
	    }
	/*if (idx != this->nKeys)	
		for (i=idx; i<this->nKeys; i++)
			{
			bpt_i_CopyKey(this, idx+1, this, idx);
			if (this->IsLeaf)
				this->Children[idx+1].Ref = this->Children[idx].Ref;
			else
				this->Children[idx+2].Child = this->Children[idx+1].Child;
			}*/
    /** Set it **/
    this->nKeys++;
    this->Keys[ idx ].Length = key_len;
    this->Keys[ idx ].Value = copy;
    if(this->IsLeaf)
        {
	this->Children[ idx ].Ref = (void*)data;
	//printf("Leaf pair(%s,%s)\n", this->Keys[idx].Value, (char*) this->Children[idx].Ref);
        }
    else
        {
        this->Children[ idx + 1 ].Child = (pBPTree)data;
        this->Children[ idx + 1 ].Child->Parent = this;
        }

    return 0;
    }

void
bpt_i_Enqueue(pBPTree this)
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


void
bpt_i_CopyKey(pBPTree tree1, int idx1, pBPTree tree2, int idx2)
        {
        tree1->Keys[idx1].Value = tree2->Keys[idx2].Value;
        tree1->Keys[idx1].Length = tree2->Keys[idx2].Length;
        }

int
bpt_i_GetNeighborIndex(pBPTree this)
        {
        int i;

        if (this == NULL)
                return -2;
        else if (this->Parent == NULL)
                return -3;

        for (i=0; i<=this->Parent->nKeys; i++)
                if (this->Parent->Children[i].Child == this)
                        return i-1;
        return -4;
        }

pBPTree
bpt_i_RemoveEntryFromNode(pBPTree this, char* key, int key_len, pBPTreeVal ptr)
        {
        int i, num_pointers;

/*	num_pointers = this->IsLeaf ? this->nKeys : this->nKeys + 1;
        i = 0;

        if (this->IsLeaf)                                                                                                                                                                                                          {
                while (this->Keys[i].Value != key)
                        i++;                                                                                                                                                                                                       for (++i; i<num_pointers; i++)
                        this->Children[i-1].Ref = this->Children[i].Ref;
                for (i=num_pointers; i<BPT_SLOTS; i++)                                                                                                                                                                                     this->Children[i].Ref = NULL;
                        this->Children[i-1].Ref = this->Children[i].Ref;
                for (i=num_pointers; i<BPT_SLOTS; i++)                                                                                                                                                                                     this->Children[i].Ref = NULL;
		}                                                                                                                                                                                                          else                                                                                                                                                                                                                       {
                while (this->Children[i].Child != (pBPTree) ptr)
                        i++;
                for (++i; i<num_pointers; i++)
                        this->Children[i-1].Child = this->Children[i].Child;
               	for (i=num_pointers; i<BPT_SLOTS+1; i++)
                        this->Children[i].Child = NULL;
		}
*/

	i = 0;
	while (bpt_i_Compare(key, key_len, this->Keys[i].Value, this->Keys[i].Length) != 0)
                i++;
        for (++i; i<this->nKeys; i++)
                bpt_i_CopyKey(this, i-1, this, i);

	num_pointers = this->IsLeaf ? this->nKeys : this->nKeys + 1;
        i = 0;

	if (this->IsLeaf)
                {
                while (this->Children[i].Ref != (void*)ptr)
                        i++;
                for (++i; i<num_pointers; i++)
                        this->Children[i-1].Ref = this->Children[i].Ref;
                }
        else
                {
                while (this->Children[i].Child != (pBPTree)ptr)
                        i++;
                for (++i; i<num_pointers; i++)
                        this->Children[i-1].Child = this->Children[i].Child;
                }
	
	this->nKeys--;

        if (this->IsLeaf)
                for (i=num_pointers; i<BPT_SLOTS; i++)
                        this->Children[i].Ref = NULL;
        else
                for (i=num_pointers; i<BPT_SLOTS+1; i++)
                        this->Children[i].Child = NULL;

        return this;
        }

pBPTree
bpt_i_AdjustRoot(pBPTree this)
        {
	if (this == NULL)
		return NULL;
	
        pBPTree new_root;
	
        if (this->nKeys > 0)
                return this;
	
        if (!this->IsLeaf)
                {
                new_root = this->Children[0].Child;
                new_root->Parent = NULL;
                }
        else
                new_root = NULL;
	
        nmFree(this, sizeof(BPTree));
	
        return new_root;
	}

pBPTree
bpt_i_CoalesceNodes(pBPTree root, pBPTree this, pBPTree neighbor, int neighbor_index, pBPTree k_prime, int k_prime_index)
        {
        int i, j, neighbor_insertion_index, this_end;
        pBPTree temp;

        if (neighbor_index == -1)
                {
                temp = this;
                this = neighbor;
                neighbor = temp;
                }

        neighbor_insertion_index = neighbor->nKeys;

        if (!this->IsLeaf)
                {
                bpt_i_CopyKey(neighbor, neighbor_insertion_index, k_prime, k_prime_index);
                neighbor->nKeys++;

                this_end = this->nKeys;
                i = neighbor_insertion_index + 1;
                for (j=0; j<this_end; j++)
                        {
                        bpt_i_CopyKey(neighbor, i, this, j);
                        neighbor->Children[i].Child = this->Children[j].Child;
                        neighbor->nKeys++;
                        this->nKeys--;
                        i++;
                        }
                neighbor->Children[i].Child = this->Children[j].Child;

                for (i=0; i<neighbor->nKeys+1; i++)
                        {
                        temp = neighbor->Children[i].Child;
                        temp->Parent = neighbor;
                        }
                }
        else
                {
                i = neighbor_insertion_index;
                for (j=0; j<this->nKeys; j++)
                        {
                        bpt_i_CopyKey(neighbor, i, this, j);
                        neighbor->Children[i].Ref = this->Children[j].Ref;
			neighbor->nKeys++;
                        i++;
                        }
                neighbor->Children[BPT_SLOTS-1].Ref = this->Children[BPT_SLOTS-1].Ref;
                }
        root = bpt_i_DeleteEntry(root, this->Parent, k_prime->Keys[k_prime_index].Value, k_prime->Keys[k_prime_index].Length, (pBPTreeVal) this);
        bptFree(this);
        return root;
        }

pBPTree
bpt_i_RedistributeNodes(pBPTree root, pBPTree this, pBPTree neighbor, int neighbor_index, pBPTree k_prime, int k_prime_index)
        {
        int i;
        pBPTree temp;

        if (neighbor_index != -1)
                {
                if (!this->IsLeaf)
                        this->Children[this->nKeys+1].Child = this->Children[this->nKeys].Child;
                /*for (i=this->nKeys; i>0; i--)
 *                         {
 *                                                 bpt_i_CopyKey(this, i, this, i-1);
 *                                                                         this->Children[i] = this->Children[i-1];
 *                                                                                                 }*/
                if (!this->IsLeaf)
                        {
                        for (i=this->nKeys; i>0; i--)
                                {
                                bpt_i_CopyKey(this, i, this, i-1);
                                this->Children[i].Child = this->Children[i-1].Child;
                                }

                        this->Children[0].Child = neighbor->Children[neighbor->nKeys].Child;
                        temp = this->Children[0].Child;
                        temp->Parent = this;
                        neighbor->Children[neighbor->nKeys].Child = NULL;
                        bpt_i_CopyKey(this, 0, k_prime, k_prime_index);
                        bpt_i_CopyKey(this->Parent, k_prime_index, neighbor, neighbor->nKeys-1);
                        }
                else
                        {
                        for (i=this->nKeys; i>0; i--)
                                {
                                bpt_i_CopyKey(this, i, this, i-1);
				this->Children[i].Ref = this->Children[i-1].Ref;
                                }

                        this->Children[0].Ref = neighbor->Children[neighbor->nKeys-1].Ref;
                        neighbor->Children[neighbor->nKeys-1].Ref = NULL;
                        bpt_i_CopyKey(this, 0, neighbor, neighbor->nKeys-1);
                        bpt_i_CopyKey(this->Parent, k_prime_index, this, 0);
                        }
	}

        else
                {
                if (this->IsLeaf)
                        {
                        bpt_i_CopyKey(this, this->nKeys, neighbor, 0);
                        this->Children[this->nKeys].Ref = neighbor->Children[0].Ref;
                        bpt_i_CopyKey(this->Parent, k_prime_index, neighbor, 1);

                        for (i=0; i<neighbor->nKeys-1; i++)
				{
                                bpt_i_CopyKey(neighbor, i, neighbor, i+1);
                                neighbor->Children[i].Ref = neighbor->Children[i+1].Ref;
                                }
                        }
                else
                        {
                        bpt_i_CopyKey(this, this->nKeys, k_prime, k_prime_index);
                        this->Children[this->nKeys+1].Child = neighbor->Children[0].Child;
                        temp = this->Children[this->nKeys+1].Child;
                        temp->Parent = this;
                        bpt_i_CopyKey(this->Parent, k_prime_index, neighbor, 0);

                        for (i=0; i<neighbor->nKeys-1; i++)
                                {
                                bpt_i_CopyKey(neighbor, i, neighbor, i+1);
                                neighbor->Children[i].Child = neighbor->Children[i+1].Child;
                                }
                        }
                if (!this->IsLeaf)
                        neighbor->Children[i].Child = neighbor->Children[i+1].Child;

                }

        this->nKeys++;
        neighbor->nKeys--;

        return root;
        }

pBPTree
bpt_i_DeleteEntry(pBPTree root, pBPTree this, char* key, int key_len, pBPTreeVal ptr)
        {
        int min_keys, neighbor_index, k_prime_index, capacity;
        pBPTree neighbor, k_prime;

        this = bpt_i_RemoveEntryFromNode(this, key, key_len, ptr);

        if (this == root)
                return bpt_i_AdjustRoot(root);

        min_keys = ceil((BPT_SLOTS + 1) / 2.0) - 1; //might want to follow their conditional assignment suggestion with cut fcn?

        if (this->nKeys >= min_keys)
                return root;

        neighbor_index = bpt_i_GetNeighborIndex(this);
        k_prime_index = neighbor_index == -1 ? 0 : neighbor_index;
        k_prime = this->Parent;
        neighbor = neighbor_index == -1 ? this->Parent->Children[1].Child : this->Parent->Children[neighbor_index].Child;

        capacity = this->IsLeaf ? BPT_SLOTS + 1 : BPT_SLOTS; //math correct for this implementation?

        if ((neighbor->nKeys + this->nKeys) < capacity)
                return bpt_i_CoalesceNodes(root, this, neighbor, neighbor_index, k_prime, k_prime_index);
        else
                return bpt_i_RedistributeNodes(root, this, neighbor, neighbor_index, k_prime, k_prime_index);
        }







		/*** bptAdd() - add a key/value pair to the tree.  Returns 1 if the
		 *** key/value pair already exists, 0 on success, or -1 on error.
		 ***/
		int
		bptAdd(pBPTree *this, char* key, int key_len, void* data)
		{
		    pBPTree node;
		    pBPTree parent = NULL, leftNode = NULL, rightNode = NULL, insertNode = NULL, node_to_prop = NULL;
		    BPTreeKey value;
		    int dx, insertIdx;
			//printf("1\n");

		   if( *this == NULL ) {
                        *this = bptNew();                                                                                                                                                                                              }

		//printf("2\n");

		    if(this == NULL || key == NULL || key_len == 0 || data == NULL){
			return -1;
		    }
			//bpt_PrintTreeSmall(*this);
		    			
			/** See if it is there. **/
		//	printf("3\n");
			int tmp = bpt_i_Find(*this, key, key_len, &node, &dx); 
			if (tmp == 0)
			    	{
			    /** Already exists.  Don't add. **/
			    	printf("already exists\n");
				return 1;
			 	}
		/*	else if(tmp == -1)
				{
				printf("bad\n");
				exit(1);
				}*/
		//	printf("4\n");
			/** Not enough room? **/
			insertNode = node;
			if (node->nKeys == BPT_SLOTS)
			    {
				leftNode = node;
				rightNode = bpt_i_Split(leftNode, CEIL_HALF_OF_LEAF_SLOTS);

			    /** Error condition if new_node is NULL. **/
			    if (!rightNode)
				return -1;
			    }
		//	printf("5\n");
			/** Which node are we adding to? **/
			if (rightNode){
			    	if (dx > CEIL_HALF_OF_LEAF_SLOTS){
					insertNode = rightNode;
					dx -= CEIL_HALF_OF_LEAF_SLOTS;
				}
			}
		//	printf("6\n");
			/** Insert the item **/
		//	printf("About to insert data - %s\n", (char*) data);	
			if (bpt_i_Insert(insertNode, key, key_len, (pBPTreeVal)data, dx) < 0){
				return -1;
			}
		//	printf("7\n");
			if(!rightNode){
				return 0;
				// because no split -- done
			}
		//	printf("8\n");
			node_to_prop = rightNode;
			value = rightNode->Keys[0];

			// go up the tree, as needed

			parent = node->Parent;
			while(1){
				if(parent == NULL){
					// add new root
					pBPTree newRoot = bptNew();
					newRoot->IsLeaf = 0;
					newRoot->nKeys = 0;

					bpt_i_Insert(newRoot, value.Value, value.Length, (pBPTreeVal)node_to_prop, 0);

					newRoot->Children[0].Child = leftNode;
					leftNode->Parent = newRoot;

					newRoot->Children[1].Child = rightNode;
					rightNode->Parent = newRoot;

					*this = newRoot;

					break;
				}
				else if(parent->nKeys == IDX_SLOTS){
					leftNode = parent;
					bpt_i_Scan(leftNode, key, key_len, &dx);
					BPTreeKey val = value;

					BPTree *rightNodeLeftChild;

					if(dx == CEIL_HALF_OF_IDX_SLOTS){
						value.Value = key;
						value.Length = key_len;
						rightNodeLeftChild = (BPTree*)node_to_prop;
						rightNode = bpt_i_Split(leftNode, CEIL_HALF_OF_IDX_SLOTS);



					}
					else if(dx < CEIL_HALF_OF_IDX_SLOTS){
						value = leftNode->Keys[CEIL_HALF_OF_IDX_SLOTS - 1];
						rightNodeLeftChild = leftNode->Children[CEIL_HALF_OF_IDX_SLOTS].Child;
						rightNode = bpt_i_Split(leftNode, CEIL_HALF_OF_IDX_SLOTS);

						leftNode->nKeys--;
					}

					else{
						value = leftNode->Keys[CEIL_HALF_OF_IDX_SLOTS];
						rightNodeLeftChild = leftNode->Children[CEIL_HALF_OF_IDX_SLOTS + 1].Child;
						rightNode = bpt_i_Split(leftNode, CEIL_HALF_OF_IDX_SLOTS + 1);
						leftNode->nKeys--;

					}

					rightNode->Children[0].Child = rightNodeLeftChild;
					rightNodeLeftChild->Parent = rightNode;

					if(dx != CEIL_HALF_OF_IDX_SLOTS){

						insertNode = leftNode;
						if(rightNode){
							if(dx > CEIL_HALF_OF_IDX_SLOTS){
								insertNode = rightNode;
								dx-= (CEIL_HALF_OF_IDX_SLOTS+1);
							}

						}
		                bpt_i_Insert(insertNode, val.Value, val.Length, (pBPTreeVal)node_to_prop, dx);

					}
					node_to_prop = rightNode;
					parent = parent->Parent;
				}
				else {
					bpt_i_Scan(parent, value.Value, value.Length, &insertIdx);
					bpt_i_Insert(parent, value.Value, value.Length, (pBPTreeVal)node_to_prop, insertIdx);
					break;

				}

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
		char* str = (char*) node->Children[idx].Ref;
		//printf("FOUND: %s\n", str);
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
	pBPTree key_leaf = NULL;
	void* key_record = NULL;
	int idx = -1, check;
	//printf("A\n");
	check = bpt_i_Find(this, key, key_len, &key_leaf, &idx);
	//printf("B\n");
	if (check == -1)
		return -1;
	//printf("C\n");
	//key_record = bptLookup(this, key, key_len);
	//printf("D\n");
	//if (key_record == NULL)
	//	printf("NULL1\n");
	if (key_leaf == NULL)
                printf("NULL2\n");
	//if (key_record == NULL || key_leaf == NULL)
	//	return -1;
	//printf("E\n");
	this = bpt_i_DeleteEntry(this, key_leaf, key, key_len, (pBPTreeVal) key_record);
//	free(key_record); //which free should i use or was this already freed
	//printf("F\n");
	return 0;
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
	    }

	/** Reset the root node **/
	if (!this->Parent)
	    bptInit(this);

    return 0;
    }

int
bpt_PrintTree(pBPTree *tree)
        {
					#define VAL_WIDTH ( 3 + 1 )
			    #define MIN_SPACING 2 //this isn't the only place to change things
			    #define NODE_PRINT_WIDTH ( ORDER * VAL_WIDTH )

		            if(*tree == NULL) { return 1; }

			    pBPTree leftmost = *tree;


			    pBPTree curr_node = leftmost;
			    int level = 0;
			    while( leftmost )
			    {
			        level++;
			        printf( "LEVEL %d: ", level );
			        while( curr_node )
			        {
			            if( curr_node->Parent != NULL
			            && (pBPTree)curr_node->Parent->Children[ 0 ].Child == curr_node
			            && curr_node != leftmost)
			            {
			            printf( "     ");
			            }

			            int limit = curr_node->IsLeaf ? BPT_SLOTS : IDX_SLOTS;
				    int i;
			            for(i = 0; i < limit; i++)
			            {
			                if( i == 0 )
			                {
			                    printf("|");
			                }
			                if( i < curr_node->nKeys)
			                {
			                    if( curr_node->IsLeaf )
			                    {
						 //for the test, every key I put in was null terminated so assuming that here.
			                       // printf( "%s-%d|", curr_node->Keys[ i ].Value, *((int*)curr_node->Children[ i + 1 ].Ref ) );
			                       printf("%s - * |", curr_node->Keys[i].Value);
			                    }
			                    else
			                    {
			                        //for the test, every key I put in was null terminated so assuming that here.
			                        printf( "%s|", curr_node->Keys[ i ].Value);
			                    }

			                }
			                else
			                {
			                    printf( "__|" );
			                }
			            }
			            if( curr_node->Next != NULL )
			            {
			                printf("--->");
			            }

			            curr_node = curr_node->Next;
			        }

			        if( curr_node == NULL )
			        {
			            if( leftmost->IsLeaf == 1 )
			            {
			                break;
			            }
			            else
			            {
			                leftmost = leftmost->Children[ 0 ].Child;
			            }
			        curr_node = leftmost;
			        }
			        printf( "\n" );
			    }
			    printf("\n");
					return 0;
    }

int
bpt_PrintTreeSmall(pBPTree root)
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
			{
                        printf("%s~", curr->Keys[i].Value);
			if (curr->IsLeaf)
				printf("(%s) ", (char*)curr->Children[i].Ref);
			}
                if (!curr->IsLeaf)
                        for (i=0; i<=curr->nKeys; i++)
                                bpt_i_Enqueue(curr->Children[i].Child);
                printf("| ");
                }
        printf("\n");
	return 0;
    }


pBPTree
bptBulkLoad(char* fname, int num)
	{
	//printf("STARTING\n");
	pBPTree root = NULL;
	int i;
	FILE* data = NULL;
	data = fopen(fname, "r");
	//printf("OPENED\n");
	char key[10], leaf[50];
	char* info;
	char* key_val;
	int leaf_sz;

	for (i=0; i<num; i++)
		{
		fscanf(data, "%s %[^\n]", key, leaf);
		//printf("%s\t%s\n", key, leaf);
		leaf_sz = strlen( leaf ) + 1; 
		info = malloc( sizeof( char ) * leaf_sz );
		strncpy( info, leaf, leaf_sz );
		bptAdd(&root, key, strlen(key), info);
		/*if (bptAdd(&root, key, strlen(key), info) != 0)
			{
			printf("NOT ADDED\n");
			return NULL;
			}*/
		}
	fclose(data);
	//printf("RETURNING\n");
	return root;
	}
