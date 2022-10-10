/* trie.c - code for trie data structure.
 * 1) DO NOT rename this file
 * 2) DO NOT add main() to this file. Create a separate file to write the driver main function.
 * */

/*
 * Andrew Murphy
 * CS367
 * To implement the trie data structure I used the following online resources.
 * # https://www.geeksforgeeks.org/trie-insert-and-search/ for the everything but deletion
 * # https://www.geeksforgeeks.org/trie-delete/ for the deletion
 * I read through the breakdowns and looked at the c++ code to get an idea of how to implement
 * it since it has been a while since I have done any coding, let alone in C.
 */

#include "trie.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>
#include <string.h>

#define CHAR_TO_INDEX(c) ((int)c -(int)'a')
#define ALPHABET_SIZE (26)

/**
 * Create an empty trie
 *
 * @return Pointer to the root node.
 *
 */
trie_node *trie_create(void) {
	trie_node *node = NULL;

	node = malloc(sizeof(trie_node));
	if (node)
	{
		int i;
		node -> isEndOfWord = 0;

		for (i = 0; i < ALPHABET_SIZE; i++)
		{
			node->children[i] = NULL;
		}
	}
	return node;
}

/**
 * Insert a word in the given trie.
 * 
 * @param root node of a trie
 * @param word to insert
 * @return 1 on success, 0 on failure. If the word already exists, return 1.
 */
int trie_insert(trie_node *root, char *word, unsigned int word_len) {
	int level;
	int index;

	trie_node *p_crawl = root;
	for (level = 0; level < word_len; level++)
	{
		index = CHAR_TO_INDEX(word[level]);
		if (!p_crawl->children[index])
		{
			p_crawl->children[index] = (struct trie_node *) trie_create();
		}
		p_crawl = (trie_node *) p_crawl->children[index];
	}
	p_crawl->isEndOfWord = true;
	return 1;
}


/**
 * Search a word in the given trie. Return 1 if word exists, otherwise return 0.
 *
 * @param root node of a trie
 * @param word Word to search in the trie
 * @param word_len Length of the word
 * @return 1 if the word exists in the trie, otherwise returns 0
 */
int trie_search(trie_node *root, char *word, unsigned int word_len) {
	int level;
	int index;
	trie_node *p_crawl = root;

	for (level = 0; level < word_len; level++)
	{
		index = CHAR_TO_INDEX(word[level]);
		if (!p_crawl->children[index])
		{
			return 0;
		}
		p_crawl = (trie_node *) p_crawl->children[index];
	}
	return (p_crawl->isEndOfWord);
}

/*
 * Helper function to see if a trie_node has children
 */
bool isEmpty(trie_node *root) {
	for (int i = 0; i < ALPHABET_SIZE; i++)
	{
		if (root->children[i])
		{
			return false;
		}
	}
	return true;
}

/*
 * Helper function for delete so I can keep track of the depth during recursion
 */
trie_node* delete_helper(trie_node *root, char *word, int depth) {
        if (!root)
        {
                return NULL;
        }
        if (depth == strlen(word))
        {
                if (root->isEndOfWord)
                {
                        root->isEndOfWord = false;
                }
                if (isEmpty(root))
                {
                        free(root);
                        root = NULL;
                }
                return root;
        }
        int index = CHAR_TO_INDEX(word[depth]);
        root->children[index] = (struct trie_node *) delete_helper((trie_node *) root->children[index], word, depth + 1);
        if (isEmpty(root) && root->isEndOfWord == false)
        {
                free(root);
                root = NULL;
        }
        return root;
}


/**
 * Delete a word in the given trie.
 *
 * @param root node of a trie
 * @param word Word to search in the trie
 * @param word_len Length of the word
 */
void trie_delete(trie_node *root, char *word, unsigned int word_len) {
	delete_helper(root, word, 0);
}
