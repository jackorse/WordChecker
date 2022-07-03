#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdint.h>
#include <limits.h>

#define RED 0
#define BLACK 1
#define ALPHABET_LENGTH 64
#define MIN_HASH_TABLE_LENGTH 50
#define MAX_HASH_TABLE_LENGTH 100000
#define HASH_TABLE_LENGTH_FACTOR 11

int num_filtered_nodes;
int hash_table_length;

typedef struct node {
    struct node *parent;
    struct node *left;
    struct node *right;
    char *word;
    char color;
    char deleted;
} node_t;

typedef struct tree {
    node_t *root;
    node_t *nil;
} RB_tree;

int k;
RB_tree dictionary;

static inline int _strcmp(const char s1[k], const char s2[k]) {
    /*for (int i = 0; i < k; i++) {
        if (s1[i] != s2[i])
            return s1[i] - s2[i];
    }
    return 0;*/
    return strcmp(s1, s2);
}

void check_tree(const node_t *x) {
    if (!x)return;
    if (x->left == NULL && x->right != NULL) {
        printf("NOOO");
    }

    check_tree(x->left);
    check_tree(x->right);
}

int compare_tree(const node_t *x1, const node_t *x2) {
    if (!x1 && !x2)return 0;
    if (!x1->word && !x2->word)return 0;
    if (strcmp(x1->word, x2->word) != 0) {
        return 1;
    }
    if (x1->color != x2->color) {
        return 1;
    }
    return compare_tree(x1->left, x2->left) + compare_tree(x1->right, x2->right);
}

void left_rotate(RB_tree *tree, node_t *x) {
    node_t *y = x->right;
    x->right = y->left; //il sottoalbero sinistro di y diventa quello destro di x
    if (y->left != tree->nil)
        y->left->parent = x;
    y->parent = x->parent; //attacca il padre di x a y
    if (x->parent == tree->nil)
        tree->root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;
    y->left = x; //mette x a sinistra di y
    x->parent = y;
}

void right_rotate(RB_tree *tree, struct node *x) {
    node_t *y = x->left;
    x->left = y->right; //il sottoalbero sinistro di y diventa quello destro di x
    if (y->right != tree->nil)
        y->right->parent = x;
    y->parent = x->parent; //attacca il padre di x a y
    if (x->parent == tree->nil)
        tree->root = y;
    else if (x == x->parent->right)
        x->parent->right = y;
    else
        x->parent->left = y;
    y->right = x; //mette x a sinistra di y
    x->parent = y;
}

void insert_fixup(RB_tree *tree, node_t *z) {
    if (z == tree->root)
        tree->root->color = BLACK;
    else {
        node_t *x = z->parent; // x e' il padre di z
        if (x->color == RED) {
            if (x == x->parent->left) {// se x e' figlio sin.ì
                node_t *y = x->parent->right; // y e' fratello di x
                if (/*y && */y->color == RED) {
                    x->color = BLACK; // Caso 1
                    y->color = BLACK; // Caso 1
                    x->parent->color = RED; // Caso 1
                    insert_fixup(tree, x->parent); // Caso 1
                } else {
                    if (z == x->right) {
                        z = x; // Caso 2
                        left_rotate(tree, z); // Caso 2
                        x = z->parent; // Caso 2
                    }
                    x->color = BLACK; // Caso 3
                    x->parent->color = RED; // Caso 3
                    right_rotate(tree, x->parent); // Caso 3
                }
            } else {//(come 6 - 18, scambiando “right”↔“left”)
                node_t *y = x->parent->left; // y e' fratello di x
                if (/*y && */y->color == RED) {
                    x->color = BLACK; // Caso 1
                    y->color = BLACK; // Caso 1
                    x->parent->color = RED; // Caso 1
                    insert_fixup(tree, x->parent); // Caso 1
                } else {
                    if (z == x->left) {
                        z = x; // Caso 2
                        right_rotate(tree, z); // Caso 2
                        x = z->parent; // Caso 2
                    }
                    x->color = BLACK; // Caso 3
                    x->parent->color = RED; // Caso 3
                    left_rotate(tree, x->parent); // Caso 3
                }
            }
        }
    }
}

void insert(RB_tree *tree, node_t *z) {
    node_t *y = tree->nil; // y padre del nodo considerato
    node_t *x = tree->root; // nodo considerato
    while (x != tree->nil) {
        y = x;
        if (_strcmp(z->word, x->word) < 0)
            x = x->left;
        else
            x = x->right;
    }
    z->parent = y;
    if (y == tree->nil)
        tree->root = z; //l'albero T e' vuoto
    else if (_strcmp(z->word, y->word) < 0)
        y->left = z;
    else
        y->right = z;
    z->left = tree->nil;
    z->right = tree->nil;
    z->color = RED;
    insert_fixup(tree, z);
}

void printTree(const node_t *x) {
    if (x->word) {
        printTree(x->left);
        if (!x->deleted)printf("%s\n", x->word);
        printTree(x->right);
    }
}

/*
char *readWord(int n) {
    char *c = malloc(sizeof(char[n]));
    for (int i = 0; i < n; i++) {
        c[i] = getchar();
        //TODO: controlli sui char
    }
    if (getchar() == '\n')
        return c;
    return ""; //TODO: error?
}*/

node_t *search(node_t *x, char *word) {
    if (!x->word || _strcmp(word, x->word) == 0)
        return x;
    if (_strcmp(word, x->word) < 0)
        return search(x->left, word);
    else return search(x->right, word);
}

node_t *tree_minimum(node_t *x) {
    while (x->left->word)
        x = x->left;
    return x;
}

node_t *tree_successor(node_t *x) {
    if (x->right->word)
        return tree_minimum(x->right);
    node_t *y = x->parent;
    while (y->word && x == y->right) {
        x = y;
        y = y->parent;
    }
    return y;
}

void delete_fixup(RB_tree *tree, node_t *x) {
    if (x->color == RED || x->parent == tree->nil)
        x->color = BLACK; // Caso 0
    else if (x == x->parent->left) {// x e' figlio sinistro
        node_t *w = x->parent->right; // w e' fratello di x
        if (/*w && */w->color == RED) {
            w->color = BLACK; // Caso 1
            x->parent->color = RED; // Caso 1
            left_rotate(tree, x->parent);// Caso 1
            w = x->parent->right;// Caso 1
        }
        if (w->left->color == BLACK && w->right->color == BLACK) {
            w->color = RED; // Caso 2
            delete_fixup(tree, x->parent); // Caso 2
        } else {
            if (w->right->color == BLACK) {
                w->left->color = BLACK; // Caso 3
                w->color = RED; // Caso 3
                right_rotate(tree, w); // Caso 3
                w = x->parent->right; // Caso 3
            }
            w->color = x->parent->color; // Caso 4
            x->parent->color = BLACK; // Caso 4
            w->right->color = BLACK; // Caso 4
            left_rotate(tree, x->parent); // Caso 4
        }
    } else {
        node_t *w = x->parent->left; // w e' fratello di x
        if (w && w->color == RED) {
            w->color = BLACK; // Caso 1
            x->parent->color = RED; // Caso 1
            right_rotate(tree, x->parent);// Caso 1
            w = x->parent->left;
        }// Caso 1
        if (w->right->color == BLACK && w->left->color == BLACK) {
            w->color = RED; // Caso 2
            delete_fixup(tree, x->parent); // Caso 2
        } else {
            if (w->left->color == BLACK) {
                w->right->color = BLACK; // Caso 3
                w->color = RED; // Caso 3
                left_rotate(tree, w); // Caso 3
                w = x->parent->left; // Caso 3
            }
            w->color = x->parent->color; // Caso 4
            x->parent->color = BLACK; // Caso 4
            w->left->color = BLACK; // Caso 4
            right_rotate(tree, x->parent); // Caso 4
        }
    }
}

/*
node_t *delete(RB_tree *T, node_t *z) {
    node_t *y;
    if (z->left == T->nil || z->right == T->nil)
        y = z;
    else
        y = tree_successor(z);
    node_t *x;
    if (y->left != T->nil)
        x = y->left;
    else x = y->right;
    x->parent = y->parent; // x potrebbe essere T.nil;
    if (y->parent == T->nil)
        T->root = x;
    else if (y == y->parent->left)
        y->parent->left = x;
    else y->parent->right = x;
    if (y != z)
        strcpy(z->word, y->word);
    //y->word = "DELET";
    if (y->color == BLACK && x)
        delete_fixup(T, x);
    return y;
}*/

node_t *delete(RB_tree *T, node_t *z) {
    node_t *y;
    if (z->left == T->nil || z->right == T->nil)
        y = z;
    else
        y = tree_successor(z);
    node_t *x;
    if (y->left != T->nil)
        x = y->left;
    else x = y->right;
    x->parent = y->parent; // x potrebbe essere T.nil;
    if (y->parent == T->nil)
        T->root = x;
    else if (y == y->parent->left)
        y->parent->left = x;
    else y->parent->right = x;
    char y_color = y->color;;
    if (y != z) {
        if (z->parent == T->nil)
            T->root = y;
        else if (z == z->parent->left)
            z->parent->left = y;
        else z->parent->right = y;
        y->parent = z->parent;
        y->left = z->left;
        y->left->parent = y;
        y->right = z->right;
        y->right->parent = y;
        y->color = z->color;
        //strcpy(z->word, y->word);
    }
    //y->word = "DELET";
    if (y_color == BLACK)
        delete_fixup(T, x);
    return z;
}

node_t *copy(const RB_tree *tree, const node_t *x, node_t *p) {
    if (x != dictionary.nil) {
        node_t *node = malloc(sizeof(node_t));
        node->color = x->color;
        node->parent = p;
        node->word = x->word;
        node->left = copy(tree, x->left, node);
        node->right = copy(tree, x->right, node);
        return node;
    }
    return tree->nil;
}

typedef struct list_node {
    node_t *ptr;
    struct list_node *next;
} list_node_t;
typedef list_node_t *list;


static inline int streq(const char *s1, const char *s2, int l) {
    for (l = l - 1; l >= 0; l--) {
        if (s1[l] != s2[l])
            return 0;
    }
    return 1;
}

static inline int _hash(node_t *ptr) {
    return (((uintptr_t) ptr) >> 5) % hash_table_length;
}


static inline void add_to_del_list(list toDelete[hash_table_length], node_t *ptr) {
    int hash = _hash(ptr);
    list index = toDelete[hash];
    list_node_t *prev = NULL;
    while (index && ptr > index->ptr) {
        /*if (streq(index->word, x->word, k)) {
            found = 1;
            break;
        }*/
        prev = index;
        index = index->next;
    }
    //if (!found) {
    //printf("Adding %s", x->word);
    if (!prev) {
        list_node_t *temp = index;
        toDelete[hash] = malloc(sizeof(list_node_t));
        toDelete[hash]->next = temp;
        toDelete[hash]->ptr = ptr;
    } else {
        list_node_t *new = malloc(sizeof(list_node_t));
        new->next = index;
        new->ptr = ptr;
        prev->next = new;
    }
}


static inline int hash(char c) {
    if (c == '_')return 0;
    if (c == '-')return 1;
    if (c >= '0' && c <= '9') return c - 46;    //2-11
    if (/*c >= 'A' &&*/ c <= 'Z') return c - 53;    //12-37
    if (/*c >= 'a' &&*/ c <= 'z') return c - 59;    //38-63
    return -1;
}

static inline char dehash(int i) {
    if (i == 0)return '_';
    if (i == 1)return '-';
    if (i >= 2 && i <= 11) return (char) (i + 46);    //2-11
    if (i >= 12 && i <= 37) return (char) (i + 53);    //12-37
    if (i >= 38 && i <= 63) return (char) (i + 59);    //38-63
    return -1;
}

void find_without_at(const RB_tree *const tree, node_t *x, char in_at[k], list toDelete[hash_table_length]) {
    if (x == tree->nil)return;
    if (x->left != tree->nil) find_without_at(tree, x->left, in_at, toDelete);

    if (!x->deleted)
        for (int i = 0; i < k; i++) {
            if (in_at[i] != -1 && x->word[i] != in_at[i]) {
                add_to_del_list(toDelete, x);
                break;
            }
        }
    if (x->right != tree->nil) find_without_at(tree, x->right, in_at, toDelete);
}

void find_with_at(const RB_tree *const tree, node_t *x, char c, int pos, list toDelete[hash_table_length]) {
    if (x == tree->nil) return;
    find_with_at(tree, x->left, c, pos, toDelete);
    if (!x->deleted)
        if (x->word[pos] == c) {
            //list_node_t *index = *toDelete;
            //char found = 0;
            /*while (index) {
                if (strcmp(index->word, x->word) == 0)
                    found = 1;
                index = index->next;
            }*/
            //if (!found) {
            /*list_node_t *temp = *toDelete;
            *toDelete = malloc(sizeof(list_node_t));
            (*toDelete)->next = temp;
            (*toDelete)->word = malloc(sizeof(char[k]) + 1);
            strcpy((*toDelete)->word, x->word);*/
            //}


            add_to_del_list(toDelete, x);
        }
    find_with_at(tree, x->right, c, pos, toDelete);
}

/*
void find_starting_with(const RB_tree *const tree, node_t *x, const char c, list toDelete[hash_table_length]) {
    if (x == tree->nil)return;
    char start = x->word[0];
    if (c == start)
        add_to_del_list(toDelete, x);
    if (c <= start)
        if (x->left != tree->nil) find_starting_with(tree, x->left, c, toDelete);
    if (c >= start)
        if (x->right != tree->nil) find_starting_with(tree, x->right, c, toDelete);
}*/

int ccc = 0;

void find_with(const RB_tree *const tree, node_t *x, const char to_filter[][2], const int new_occ,
               const char occ[ALPHABET_LENGTH],
               const char occ_applied[ALPHABET_LENGTH],
               list toDelete[hash_table_length]) {
    if (x == tree->nil)return;
    if (x->left != tree->nil)find_with(tree, x->left, to_filter, new_occ, occ, occ_applied, toDelete);
    if (!x->deleted)
        for (int i = 0; i < new_occ; i++) {
            int count = 0;
            char *word = x->word;
            int hashed = to_filter[i][1];
            for (int j = 0; j < k; j++) {
                ccc++;
                if (word[j] == to_filter[i][0])
                    count++;
                if (count > occ[hashed])break;
            }
            if (occ[hashed] != count) {
                add_to_del_list(toDelete, x);
                break;
            }
        }

    if (x->right != tree->nil) find_with(tree, x->right, to_filter, new_occ, occ, occ_applied, toDelete);
}

void find_with_min_occ(const RB_tree *const tree, node_t *x, char c, int min, list toDelete[hash_table_length]) {
    if (x == tree->nil) return;
    find_with_min_occ(tree, x->left, c, min, toDelete);
    if (!x->deleted) {
        int count = 0;
        for (int i = 0; i < k; i++) {
            if (x->word[i] == c)
                count++;
        }
        if (count < min) {
            //char found = 0;

            add_to_del_list(toDelete, x);
        }
    }
    find_with_min_occ(tree, x->right, c, min, toDelete);
}


int free_list(list l) {
    list_node_t *temp;
    int length = 0;
    while (l) {
        temp = l;
        l = temp->next;
        free(temp);
        length++;
    }
    return length;
}

int delete_in(RB_tree *tree, list toDelete[hash_table_length]) {
    int min = INT_MAX, max = 0, count = 0;
    for (int i = 0; i < hash_table_length; i++) {
        list_node_t *index = toDelete[i];
        while (index) {
            //node_t *t = search(tree->root, index->word);
            assert(index->ptr != tree->nil);
            //node_t *t = delete(tree, index->ptr);
            //assert(t == index->ptr);
            //free(t->word);
            //free(t);
            index->ptr->deleted = 1;

            index = index->next;
            count++;
        }
        int l = free_list(toDelete[i]);
        if (l < min) min = l;
        if (l > max)max = l;
        toDelete[i] = NULL;
    }
    //printf("MIN: %d, MAX: %d\n", min, max);
    return count;
}

int check_filters(const char word[k],
                  const char *in_at,
                  const char min_occ[ALPHABET_LENGTH],
                  const char occ[ALPHABET_LENGTH],
                  const char not_in_at[k][ALPHABET_LENGTH]) {

    for (int i = 0; i < k; i++) {
        if (in_at[i] != -1 && word[i] != in_at[i]) {
            return 0;
        }
        for (int j = 0; j < ALPHABET_LENGTH; j++) {
            if (not_in_at[i][j] && occ[j] != 0 && word[i] == dehash(j)) {
                return 0;
            }
        }
    }
    for (int i = 0; i < ALPHABET_LENGTH; i++) {
        if (occ[i] >= 0) {
            char c = dehash(i);
            int count = 0;
            for (int j = 0; j < k; j++) {
                if (word[j] == c)
                    count++;
            }
            if (count != occ[i])
                return 0;
        } else if (min_occ[i] > 0) {
            char c = dehash(i);
            int count = 0;
            for (int j = 0; j < k; j++) {
                if (word[j] == c)
                    count++;
            }
            if (count < min_occ[i])
                return 0;
        }
    }
    return 1;
}

int inserisci_inizio(char in_at[k], char min_occ[ALPHABET_LENGTH], char occ[ALPHABET_LENGTH],
                     char not_in_at[k][ALPHABET_LENGTH]) {
    node_t *new_dict;
    int count = 0;
    char read[256];
    while (1) {
        if (scanf("%s", read) < 0)break;
        if (_strcmp(read, "+inserisci_fine") == 0)
            return count;
        if (strlen(read) == k) {
            char *new_dict_word = malloc(sizeof(char[k + 1]));
            //char *new_filtered_word = malloc(sizeof(char[k + 1]));
            strcpy(new_dict_word, read);
            //strcpy(new_filtered_word, read);
            new_dict = malloc(sizeof(node_t));
            new_dict->word = new_dict_word;
            new_dict->left = dictionary.nil;
            new_dict->right = dictionary.nil;
            new_dict->deleted = 0;
            insert(&dictionary, new_dict);
            if (!check_filters(new_dict_word, in_at, min_occ, occ, not_in_at)) {
                /*new_filtered = malloc(sizeof(node_t));
                new_filtered->word = new_dict_word;
                new_filtered->left = dict->nil;
                new_filtered->right = dict->nil;
                insert(dict, new_filtered);*/
                count++;
                new_dict->deleted = 1;
            }
        }
    }
    return count;
}

/*
void apply_filters(RB_tree *const filtered_tree,
                   const char *to_filter_in_at, const char not_in[ALPHABET_LENGTH],
                   const char to_filter_min_occ[ALPHABET_LENGTH], const char occ[ALPHABET_LENGTH],
                   const char to_filter_not_in_at[k][ALPHABET_LENGTH],
                   list to_delete[hash_table_length]) {
    if (!filtered_tree->root) {
        printf("filtered_tree null!!!");
        return;
    }


    for (int i = 0; i < k; i++) {
        if (to_filter_in_at[i] >= 0) {
            char c = to_filter_in_at[i];
            find_without_at(filtered_tree, filtered_tree->root, c, i, to_delete);
            delete_in(filtered_tree, to_delete);
        }

        for (int j = 0; j < ALPHABET_LENGTH; j++) {
            if (to_filter_not_in_at[i][j] && occ[j] != 0) {
                char c = dehash(j);
                find_with_at(filtered_tree, filtered_tree->root, c, i, to_delete);
                delete_in(filtered_tree, to_delete);
            }
        }
    }
    for (int i = 0; i < ALPHABET_LENGTH; i++) {
        if (occ[i] >= 0) {
            char c = dehash(i);
            find_with(filtered_tree, filtered_tree->root, c, occ[i], to_delete);
            delete_in(filtered_tree, to_delete);
        } else if (to_filter_min_occ[i] > 0) {
            char c = dehash(i);
            find_with_min_occ(filtered_tree, filtered_tree->root, c, to_filter_min_occ[i], to_delete);
            delete_in(filtered_tree, to_delete);
        }
    }
}*/


void apply_filters(const RB_tree *const filtered_tree, node_t *x,
                   const char to_filter_occ[][2], const int new_occ,
                   const char to_filter_in_at[][2], const int new_in_at,
                   const char to_filter_min_occ[][2], const int new_min_occ,
                   const char to_filter_not_in_at[][2], const int new_not_in_at,
                   list to_delete[hash_table_length]) {

    if (x == filtered_tree->nil)return;
    if (x->left != filtered_tree->nil)
        apply_filters(filtered_tree, x->left, to_filter_occ, new_occ, to_filter_in_at, new_in_at,
                      to_filter_min_occ,
                      new_min_occ, to_filter_not_in_at, new_not_in_at, to_delete);

    char *word = x->word;
    for (int i = 0; i < k; i++) {

        /*Filtro per lettere già trovate
         * to_filter_in_at[][0] = lettera trovata
         * to_filter_in_at[][1] = posizione
         */
        if (i < new_in_at) {
            if (word[(int) to_filter_in_at[i][1]] != to_filter_in_at[i][0]) {
                add_to_del_list(to_delete, x);
                break;
            }
        }

        /*Filtro per lettere not presenti in una posizione
         * to_filter_not_in_at[][0] = lettera
         * to_filter_not_in_at[][1] = posizione
         */
        if (i < new_not_in_at) {
            if (word[(int) to_filter_not_in_at[i][1]] == to_filter_not_in_at[i][0]) {
                add_to_del_list(to_delete, x);
                break;
            }
        }


        /*Filtro per numero di occorrenze
         * to_filter_occ[][0] = lettera
         * to_filter_occ[][1] = numero occorrenze
         */
        if (i < new_occ) {
            int count = 0;
            for (int j = 0; j < k; j++) {
                ccc++;
                if (word[j] == to_filter_occ[i][0])
                    count++;
                if (count > to_filter_occ[i][1])break;
            }
            if (to_filter_occ[i][1] != count) {
                add_to_del_list(to_delete, x);
                break;
            }
        }

        /*Filtro per numero minimo di occorrenze
         * to_filter_min_occ[][0] = lettera
         * to_filter_min_occ[][1] = numero minimo occorrenze
         */
        if (i < new_min_occ) {
            int count = 0;
            for (int j = 0; j < k; j++) {
                if (word[j] == to_filter_min_occ[i][0])
                    count++;
                if (count > to_filter_min_occ[i][1])break;
            }
            if (count < to_filter_min_occ[i][1]) {
                add_to_del_list(to_delete, x);
                break;
            }
        }

    }


    if (x->right != filtered_tree->nil)
        apply_filters(filtered_tree, x->right, to_filter_occ, new_occ, to_filter_in_at, new_in_at,
                      to_filter_min_occ,
                      new_min_occ, to_filter_not_in_at, new_not_in_at, to_delete);

}

/*
void apply_filter_in_at(RB_tree *filtered_tree, const char *in_at, int k) {
    list toDelete = NULL;
    for (int i = 0; i < k; i++) {
        if (in_at[i] >= 0) {
            char c = in_at[i];
            find_without_at(filtered_tree, filtered_tree->root, c, i, &toDelete);
        }
    }
    delete_in(filtered_tree, &toDelete);
}


void apply_filter_not_in(RB_tree *filtered_tree, const char not_in[ALPHABET_LENGTH]) {
    list toDelete = NULL;
    for (int i = 0; i < ALPHABET_LENGTH; i++) {
        if (not_in[i]) {
            char c = dehash(i);
            find_with(*filtered_tree, c, &toDelete);
            delete_in(filtered_tree, &toDelete);
        }
    }
}

void apply_filter_min_occ(RB_tree *filtered_tree, const char min_occ[ALPHABET_LENGTH], int k) {
    list toDelete = NULL;
    for (int i = 0; i < ALPHABET_LENGTH; i++) {
        if (min_occ[i] > 0) {
            char c = dehash(i);
            find_with_min_occ(filtered_tree, filtered_tree->root, c, min_occ[i], k, &toDelete);
            delete_in(filtered_tree, &toDelete);
        }
    }
}
*/
int count(const RB_tree *tree, node_t *x) {
    if (x == tree->nil) return 0;
    int c = x->deleted ? 0 : 1;
    c += count(tree, x->left);
    c += count(tree, x->right);
    return c;
}

void destroy(RB_tree *tree, node_t *x) {
    if (x == tree->nil) {
        //free(tree->nil);
        return;
    }
    if (x->left != tree->nil) destroy(tree, x->left);
    if (x->right != tree->nil) destroy(tree, x->right);

    if (x == tree->root) {
        //free(tree->nil->word);
        //free(tree->nil);
    }
    //free(x->word);
    //free(x);
    x->deleted = 0;
}

int n_partite = 0;

void nuova_partita() {
    char ref_word[k + 1];
    num_filtered_nodes = count(&dictionary, dictionary.root);
    hash_table_length = num_filtered_nodes / HASH_TABLE_LENGTH_FACTOR;
    if (hash_table_length < MIN_HASH_TABLE_LENGTH)hash_table_length = MIN_HASH_TABLE_LENGTH;
    if (hash_table_length > MAX_HASH_TABLE_LENGTH)hash_table_length = MAX_HASH_TABLE_LENGTH;
    //char not_in[ALPHABET_LENGTH] = {0};
    char min_occ[ALPHABET_LENGTH] = {0};
    //char min_occ_applied[ALPHABET_LENGTH] = {0};
    char occ[ALPHABET_LENGTH];
    //char occ_applied[ALPHABET_LENGTH] = {0};
    char in_at[k];
    //char in_at_applied[k];
    char not_in_at[k][ALPHABET_LENGTH];
    //char not_in_at_applied[k][ALPHABET_LENGTH];
    for (int i = 0; i < k; i++) {
        in_at[i] = -1;
        //in_at_applied[i] = 0;
    }
    for (int i = 0; i < ALPHABET_LENGTH; i++) {
        occ[i] = -1;
    }
    for (int i = 0; i < k; i++) {
        for (int j = 0; j < ALPHABET_LENGTH; j++) {
            not_in_at[i][j] = 0;
            //not_in_at_applied[i][j] = 0;
        }
    }

    if (!scanf("%s", ref_word)) return;
    int n;
    if (!scanf("%d", &n)) return;
    //RB_tree filtered_tree = dictionary;
    /*filtered_tree.nil = malloc(sizeof(node_t));
    filtered_tree.nil->word = NULL;
    filtered_tree.nil->color = BLACK;
    filtered_tree.nil->left = NULL;
    filtered_tree.nil->right = NULL;
    filtered_tree.nil->parent = NULL;
    filtered_tree.root = filtered_tree.nil;
    filtered_tree.root = copy(&filtered_tree, dictionary.root, filtered_tree.nil);*/
    //printTree(filtered_tree);
    while (n > 0) {
        list to_delete[hash_table_length];
        for (int i = 0; i < hash_table_length; i++)
            to_delete[i] = NULL;

        char input[256];
        if (scanf("%s", input) < 0)break;
        if (_strcmp(input, "+stampa_filtrate") == 0)
            printTree(dictionary.root);
        else if (_strcmp(input, "+inserisci_inizio") == 0) {
            num_filtered_nodes += inserisci_inizio(in_at, min_occ, occ, not_in_at);
            //apply_filters(&filtered_tree, in_at,/* not_in, */min_occ, occ, not_in_at, to_delete);
        } else if (strlen(input) == k) {
            if (_strcmp(input, ref_word) == 0) {
                printf("ok\n");
                destroy(&dictionary, dictionary.root);
                return;
            } else if (search(dictionary.root, input) != dictionary.nil) {
                char _min_occ[ALPHABET_LENGTH] = {0};
                char res[k];
                char used[k];

                int new_occ = 0;
                char to_filter_occ[k][2];
                char to_filter_in_at[k][2];
                int new_in_at = 0;
                char to_filter_min_occ[k][2];
                int new_min_occ = 0;
                char to_filter_not_in_at[k][2];
                int new_not_in_at = 0;

                for (int i = 0; i < k; i++) {
                    res[i] = 0;
                    used[i] = 0;
                }
                //list toDelete = NULL;
                for (int j = 0; j < k; j++) {
                    if (ref_word[j] == input[j]) {
                        res[j] = '+';
                        used[j] = 1;

                        if (in_at[j] == -1) {
                            in_at[j] = input[j];
                            to_filter_in_at[new_in_at][0] = input[j];
                            to_filter_in_at[new_in_at][1] = j;
                            new_in_at++;
                        }

                        //find_without_at(filtered_tree, input[j], j, &toDelete);
                        /*printf("To delete:\n");
                        while (toDelete) {
                            printf("\n%s", (toDelete)->ptr->word);
                            toDelete = (toDelete)->next;
                        }*/
                        //delete_in(&filtered_tree, &toDelete);
                        _min_occ[hash(input[j])]++;
                    }
                }
                /*if (new_in_at) {
                    find_without_at(&filtered_tree, filtered_tree.root, in_at, to_delete);
                    delete_in(&filtered_tree, to_delete);
                }*/
                //apply_filter_in_at(&filtered_tree, in_at, k);

                for (int j = 0; j < k; j++) {
                    if (res[j] != '+') {
                        int found = 0;

                        for (int i = 0; i < k; i++) {
                            if (ref_word[i] == input[j] && used[i] != 1) {
                                res[j] = '|';

                                used[i] = 1;
                                found = 1;
                                _min_occ[hash(input[j])]++;

                                if (!not_in_at[j][hash(input[j])]) {
                                    not_in_at[j][hash(input[j])] = 1;
                                    to_filter_not_in_at[new_not_in_at][0] = input[j];
                                    to_filter_not_in_at[new_not_in_at][1] = j;
                                    new_not_in_at++;
                                }
                                break;
                            }
                        }
                        if (_min_occ[hash(input[j])] > min_occ[hash(input[j])]) {
                            min_occ[hash(input[j])] = _min_occ[hash(input[j])];
                            /*if (min_occ[hash(input[j])] > 0) {
                                find_with_min_occ(&filtered_tree, filtered_tree.root, input[j],
                                                                  min_occ[hash(input[j])],
                                                                  to_delete);
                                delete_in(&filtered_tree, to_delete);
                            }*/
                            to_filter_min_occ[new_min_occ][0] = input[j];
                            to_filter_min_occ[new_min_occ][1] = min_occ[hash(input[j])];
                            new_min_occ++;
                        }
                        if (!found) {
                            res[j] = '/';
                            //not_in[hash(input[j])] = 1;

                            if (occ[hash(input[j])] == -1) {
                                occ[hash(input[j])] = min_occ[hash(input[j])];
                                to_filter_occ[new_occ][0] = input[j];
                                to_filter_occ[new_occ][1] = occ[hash(input[j])];
                                new_occ++;
                            }

                            if (!not_in_at[j][hash(input[j])]) {
                                not_in_at[j][hash(input[j])] = 1;
                                if (occ[hash(input[j])] != 0) {
                                    to_filter_not_in_at[new_not_in_at][0] = input[j];
                                    to_filter_not_in_at[new_not_in_at][1] = j;
                                    new_not_in_at++;
                                }
                            }
                            /*if (!not_in_at_applied[j][hash(input[j])] && occ[hash(input[j])] != 0) {
                                find_with_at(&filtered_tree, filtered_tree.root, input[j], j, to_delete);
                                delete_in(&filtered_tree, to_delete);
                                                not_in_at_applied[j][hash(input[j])] = 1;
                                            }
                                            if (!occ_applied[hash(input[j])] && occ[hash(input[j])] >= 0) {
                                                to_filter_occ[new_occ][0] = input[j];
                                                to_filter_occ[new_occ][1] = (char) hash(input[j]);
                                                new_occ++;
                                            }
                                            */
                                            //find_with(filtered_tree, input[j], &toDelete);
                                            //delete_in(&filtered_tree, &toDelete);
                                        }
                                    }
                                }

                                for (int h = 0; h < new_occ; h++) {
                                    for (int h2 = h + 1; h2 < new_occ; h2++) {
                                        if (to_filter_occ[h][0] == to_filter_occ[h2][0]) {
                                            to_filter_occ[h2][0] = to_filter_occ[new_occ - 1][0];
                                            to_filter_occ[h2][1] = to_filter_occ[new_occ - 1][1];
                                            new_occ--;
                                        }
                                    }
                                }

                if (new_occ > 0 || new_not_in_at > 0 || new_min_occ > 0 || new_in_at > 0) {
                    apply_filters(&dictionary, dictionary.root, to_filter_occ, new_occ, to_filter_in_at,
                                  new_in_at,
                                  to_filter_min_occ, new_min_occ, to_filter_not_in_at, new_not_in_at, to_delete);
                    num_filtered_nodes -= delete_in(&dictionary, to_delete);
                }
                /*if (new_occ) {
                    find_with(&filtered_tree, filtered_tree.root, to_filter_occ, new_occ, occ, occ_applied, to_delete);
                    delete_in(&filtered_tree, to_delete);
                                }
                                for (int j = 0; j < k; j++)
                                    occ_applied[hash(input[j])] = 1;
                                * /
                                //apply_filter_not_in(&filtered_tree, not_in);
                                //apply_filter_min_occ(&filtered_tree, min_occ,k);
                                //apply_filters(&filtered_tree, in_at,/* not_in, */min_occ, occ, not_in_at, to_delete);

                            for (int j = 0; j < k; j++) {
                                printf("%c", res[j]);
                            }

                            printf("\n%d\n", num_filtered_nodes);
                            hash_table_length = num_filtered_nodes / HASH_TABLE_LENGTH_FACTOR;
                            if (hash_table_length < MIN_HASH_TABLE_LENGTH)hash_table_length = MIN_HASH_TABLE_LENGTH;
                            if (hash_table_length > MAX_HASH_TABLE_LENGTH)hash_table_length = MAX_HASH_TABLE_LENGTH;
                            n--;
                            //printf("CICLI: %d\n", ccc);
                        } else {
                            printf("not_exists\n");
                        }
                    }
                }
                printf("ko\n");;
                destroy(&dictionary, dictionary.root);
                n_partite++;
                if (n_partite >= 1)
                    exit(-1);
            }

            int main() {
                dictionary.nil = malloc(sizeof(node_t));
                dictionary.nil->word = NULL;
                dictionary.nil->color = BLACK;
                dictionary.root = dictionary.nil;
                node_t *x;
                setvbuf(stdout, NULL, _IONBF, 0);
                if (scanf("%d", &k)) {
                    char adding_words = 1;
                    char temp[256];
                    while (1) {
                        if (scanf("%s", temp) < 0)break;
                        if (_strcmp(temp, "+nuova_partita") == 0)
                            nuova_partita();
                        else if (_strcmp(temp, "+inserisci_inizio") == 0)
                            adding_words = 1;
                        else if (_strcmp(temp, "+inserisci_fine") == 0)
                            adding_words = 0;
                        else if (strlen(temp) == k && adding_words) {
                            char *c = malloc(sizeof(char[k + 1]));
                            strcpy(c, temp);
                            x = malloc(sizeof(node_t));
                            x->word = c;
                            x->deleted = 0;
                            insert(&dictionary, x);
                        }
                    }
                }
                return 0;
            }
