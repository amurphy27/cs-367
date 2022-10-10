#include <stdbool.h>

typedef struct node {
	struct trie_node *children[26];
	bool isEndOfWord;
} trie_node;


trie_node *trie_create(void);
int trie_insert(trie_node *trie, char *word, unsigned int word_len);
int trie_search(trie_node *trie, char *word, unsigned int word_len);
void trie_delete(trie_node *trie, char *word, unsigned int word_len);