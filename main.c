#ifdef EVAL
#define NDEBUG
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>

#define RED 0
#define BLACK 1
#define ALPHABET_LENGTH 64
#define NUM_NODES_PER_MALLOC_INIT 20000
#define NUM_NODES_PER_MALLOC 10000


#ifdef EVAL
char print_buffer[10000];
#endif


typedef struct node {
    struct node *parent;
    struct node *left;
    struct node *right;
    struct node *prev;
    struct node *next;
    char *word;
    char color;
    char deleted;
} node_t;

typedef struct tree {
    node_t *root;
    node_t *head;
    node_t *nil;
} RB_tree;

int k;
RB_tree dictionary;
int num_filtered_nodes = 0;

size_t malloc_word_size, malloc_node_size;
const size_t node_size = sizeof(node_t);
int read_length = 64;

static inline int _strcmp(const char s1[k], const char s2[k]) {
    for (int i = 0; i < k; i++) {
        if (s1[i] != s2[i])
            return s1[i] - s2[i];
    }
    return 0;
    //return strncmp(s1, s2, k);
}

void left_rotate(node_t *x) {
    node_t *y = x->right;
    x->right = y->left; //il sottoalbero sinistro di y diventa quello destro di x
    if (y->left != dictionary.nil)
        y->left->parent = x;
    y->parent = x->parent; //attacca il padre di x a y
    if (x->parent == dictionary.nil)
        dictionary.root = y;
    else if (x == x->parent->left)
        x->parent->left = y;
    else
        x->parent->right = y;
    y->left = x; //mette x a sinistra di y
    x->parent = y;
}

void right_rotate(node_t *x) {
    node_t *y = x->left;
    x->left = y->right; //il sottoalbero sinistro di y diventa quello destro di x
    if (y->right != dictionary.nil)
        y->right->parent = x;
    y->parent = x->parent; //attacca il padre di x a y
    if (x->parent == dictionary.nil)
        dictionary.root = y;
    else if (x == x->parent->right)
        x->parent->right = y;
    else
        x->parent->left = y;
    y->right = x; //mette x a sinistra di y
    x->parent = y;
}

void insert_fixup(node_t *z) {
    if (z == dictionary.root)
        dictionary.root->color = BLACK;
    else {
        node_t *x = z->parent; // x e' il padre di z
        if (x->color == RED) {
            if (x == x->parent->left) {// se x e' figlio sin.ì
                node_t *y = x->parent->right; // y e' fratello di x
                if (/*y && */y->color == RED) {
                    x->color = BLACK; // Caso 1
                    y->color = BLACK; // Caso 1
                    x->parent->color = RED; // Caso 1
                    insert_fixup(x->parent); // Caso 1
                } else {
                    if (z == x->right) {
                        z = x; // Caso 2
                        left_rotate(z); // Caso 2
                        x = z->parent; // Caso 2
                    }
                    x->color = BLACK; // Caso 3
                    x->parent->color = RED; // Caso 3
                    right_rotate(x->parent); // Caso 3
                }
            } else {//(come 6 - 18, scambiando “right”↔“left”)
                node_t *y = x->parent->left; // y e' fratello di x
                if (/*y && */y->color == RED) {
                    x->color = BLACK; // Caso 1
                    y->color = BLACK; // Caso 1
                    x->parent->color = RED; // Caso 1
                    insert_fixup(x->parent); // Caso 1
                } else {
                    if (z == x->left) {
                        z = x; // Caso 2
                        right_rotate(z); // Caso 2
                        x = z->parent; // Caso 2
                    }
                    x->color = BLACK; // Caso 3
                    x->parent->color = RED; // Caso 3
                    left_rotate(x->parent); // Caso 3
                }
            }
        }
    }
}

void insert(node_t *z) {
    node_t *y = dictionary.nil; // y padre del nodo considerato
    node_t *x = dictionary.root; // nodo considerato
    char *z_word = z->word;
    while (x != dictionary.nil) {
        y = x;
        if (_strcmp(z_word, x->word) < 0)
            x = x->left;
        else
            x = x->right;
    }
    z->parent = y;
    if (y == dictionary.nil)
        dictionary.root = z; //l'albero T e' vuoto
    else if (_strcmp(z_word, y->word) < 0)
        y->left = z;
    else
        y->right = z;
    z->left = dictionary.nil;
    z->right = dictionary.nil;
    z->color = RED;
    insert_fixup(z);
}

static inline void print(char *s) {
    for (int i = 0; i < k; i++)
        putchar_unlocked(s[i]);
    putchar_unlocked('\n');
}

void printTree(const node_t *x) {
    if (x == dictionary.nil)return;

    if (x->left != dictionary.nil)printTree(x->left);
    if (!x->deleted)print(x->word);
    if (x->right != dictionary.nil)printTree(x->right);
}

node_t *search(node_t *x, char *word) {
    if (x == dictionary.nil || _strcmp(word, x->word) == 0)
        return x;
    if (_strcmp(word, x->word) < 0)
        return search(x->left, word);
    else return search(x->right, word);
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
    if (/*i >= 12 &&*/ i <= 37) return (char) (i + 53);    //12-37
    if (/*i >= 38 && */i <= 63) return (char) (i + 59);    //38-63
    return -1;
}

int check_filters(const char word[k],
                  const int in_at[k][2], const int num_in_at,
                  const int min_occ[ALPHABET_LENGTH][2], const int num_min_occ,
                  const int occ[ALPHABET_LENGTH][2], const int num_occ,
                  const int not_in_at[k * ALPHABET_LENGTH][2], const int num_not_in_at) {

    for (int i = 0; i < k; i++) {
        if (i < num_in_at && in_at[i][0] != word[(int) in_at[i][1]]) {
            return 0;
        }
        for (int j = 0; i * ALPHABET_LENGTH + j < num_not_in_at && j < ALPHABET_LENGTH; j++) {
            if (not_in_at[i * ALPHABET_LENGTH + j][0] == word[(int) not_in_at[i * ALPHABET_LENGTH + j][1]]) {
                return 0;
            }
        }
    }
    for (int i = 0; i < ALPHABET_LENGTH; i++) {
        if (i < num_occ) {
            char c = occ[i][0];
            int count = 0;
            for (int j = 0; j < k; j++) {
                if (word[j] == c)
                    count++;
            }
            if (count != occ[i][1])
                return 0;
        }
        if (i < num_min_occ) {
            char c = min_occ[i][0];
            int count = 0;
            for (int j = 0; j < k; j++) {
                if (word[j] == c)
                    count++;
            }
            if (count < min_occ[i][1])
                return 0;
        }
    }
    return 1;
}

void inserisci_inizio(const char in_at[k], const char min_occ[ALPHABET_LENGTH], const char occ[ALPHABET_LENGTH],
                      const char not_in_at[k][ALPHABET_LENGTH]) {
    int _occ[ALPHABET_LENGTH][2];
    int num_occ = 0;
    int _min_occ[ALPHABET_LENGTH][2];
    int num_min_occ = 0;
    int _not_in_at[ALPHABET_LENGTH * k][2];
    int num_not_in_at = 0;
    int _in_at[k][2];
    int num_in_at = 0;


    for (int i = 0; i < k; i++) {
        if (in_at[i] != -1) {
            _in_at[num_in_at][0] = in_at[i];
            _in_at[num_in_at][1] = i;
            num_in_at++;
        }

        for (int j = 0; j < ALPHABET_LENGTH; j++) {
            if (not_in_at[i][j] && occ[j] != 0) {
                _not_in_at[num_not_in_at][0] = dehash(j);
                _not_in_at[num_not_in_at][1] = i;
                num_not_in_at++;
            }
        }
    }

    for (int i = 0; i < ALPHABET_LENGTH; i++) {
        if (occ[i] >= 0) {
            char c = dehash(i);
            _occ[num_occ][0] = c;
            _occ[num_occ][1] = occ[i];
            num_occ++;
        } else if (min_occ[i] > 0) {
            char c = dehash(i);
            _min_occ[num_min_occ][0] = c;
            _min_occ[num_min_occ][1] = min_occ[i];
            num_min_occ++;
        }
    }

    node_t *new_node;
    char read[read_length];
    char *word_buffer = malloc(malloc_word_size);
    void *node_buffer = malloc(malloc_node_size);
    int num_nodes = 0;
    while (scanf("%s", read) > 0) {
        if (strcmp(read, "+inserisci_fine") == 0)
            return;
        assert (strlen(read) == k);
        //char *new_dict_word = malloc(sizeof(char[k + 1]));
        //char *new_filtered_word = malloc(sizeof(char[k + 1]));
        if (num_nodes >= NUM_NODES_PER_MALLOC) {
            node_buffer = malloc(malloc_node_size);
            assert(node_buffer);
            word_buffer = malloc(malloc_word_size);
            assert(word_buffer);
#ifndef EVAL
            word_buffer[0] = '\0';
#endif
            num_nodes = 0;
        }
        char *word = word_buffer + num_nodes * k;
        strncpy(word, read, k);
#ifndef EVAL
        word_buffer[num_nodes * k + k] = '\0';
#endif
        new_node = node_buffer + num_nodes * node_size;
        new_node->word = word;
        insert(new_node);
        if (check_filters(read,
                          _in_at, num_in_at,
                          _min_occ, num_min_occ,
                          _occ, num_occ,
                          _not_in_at, num_not_in_at)) {
            /*new_filtered = malloc(sizeof(node_t));
            new_filtered->word = new_dict_word;
            new_filtered->left = dict->nil;
            new_filtered->right = dict->nil;
            insert(dict, new_filtered);*/
            num_filtered_nodes++;
            new_node->deleted = 0;
            node_t *temp = dictionary.head;
            dictionary.head = new_node;
            new_node->prev = NULL;
            new_node->next = temp;
            if (temp)temp->prev = new_node;
        } else
            new_node->deleted = 1;
        num_nodes++;
    }
}

void apply_filters(
        const int to_filter_occ[][2], const int new_occ,
        const int to_filter_in_at[][2], const int new_in_at,
        const int to_filter_min_occ[][2], const int new_min_occ,
        const int to_filter_not_in_at[][2], const int new_not_in_at
        /*list to_delete[hash_table_length]*/) {
    if (num_filtered_nodes <= 0)return;
    node_t *index = dictionary.head;
    while (index) {
        assert(!index->deleted);
        node_t *succ = index->next;
        char *word = index->word;
        for (int i = 0; i < k; i++) {

            /*Filtro per lettere già trovate
             * to_filter_in_at[][0] = lettera trovata
             * to_filter_in_at[][1] = posizione
             */
            if (i < new_in_at) {
                if (word[(int) to_filter_in_at[i][1]] != to_filter_in_at[i][0]) {
                    index->deleted = 1;
                    num_filtered_nodes--;
                    //add_to_del_list(to_delete, x);
                    if (!index->prev) {
                        dictionary.head = dictionary.head->next;
                        if (dictionary.head)dictionary.head->prev = NULL;
                    } else {
                        index->prev->next = index->next;
                        if (index->next)index->next->prev = index->prev;
                    }
                    //free(index);
                    break;
                }
            }

            /*Filtro per lettere not presenti in una posizione
             * to_filter_not_in_at[][0] = lettera
             * to_filter_not_in_at[][1] = posizione
             */
            if (i < new_not_in_at) {
                if (word[(int) to_filter_not_in_at[i][1]] == to_filter_not_in_at[i][0]) {
                    index->deleted = 1;
                    num_filtered_nodes--;
                    if (!index->prev) {
                        dictionary.head = dictionary.head->next;
                        if (dictionary.head)dictionary.head->prev = NULL;
                    } else {
                        index->prev->next = index->next;
                        if (index->next)index->next->prev = index->prev;
                    }
                    //free(index);
                    //add_to_del_list(to_delete, x);
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
                    if (word[j] == to_filter_occ[i][0])
                        count++;
                    if (count > to_filter_occ[i][1])break;
                }
                if (to_filter_occ[i][1] != count) {
                    index->deleted = 1;
                    num_filtered_nodes--;
                    if (!index->prev) {
                        dictionary.head = dictionary.head->next;
                        if (dictionary.head)dictionary.head->prev = NULL;
                    } else {
                        index->prev->next = index->next;
                        if (index->next)index->next->prev = index->prev;
                    }
                    //free(index);
                    //add_to_del_list(to_delete, x);
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
                    index->deleted = 1;
                    num_filtered_nodes--;
                    if (!index->prev) {
                        dictionary.head = dictionary.head->next;
                        if (dictionary.head)dictionary.head->prev = NULL;
                    } else {
                        index->prev->next = index->next;
                        if (index->next)index->next->prev = index->prev;
                    }
                    //free(index);
                    //add_to_del_list(to_delete, x);
                    break;
                }
            }
        }
        index = succ;
    }
}

void reset(node_t *x) {
    if (x == dictionary.nil) return;
    if (x->left != dictionary.nil) reset(x->left);
    if (x->right != dictionary.nil) reset(x->right);

    if (x->deleted) {
        x->deleted = 0;
        num_filtered_nodes++;
        node_t *temp = dictionary.head;
        dictionary.head = x;
        x->prev = NULL;
        x->next = temp;
        if (temp)temp->prev = x;
    }
}

void nuova_partita() {
    char ref_word[k + 1];
    //num_filtered_nodes = count(&dictionary, dictionary.root);
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
    while (n > 0) {
        char input[read_length];
        if (scanf("%s", input) < 0)break;
        if (strcmp(input, "+stampa_filtrate") == 0)
            printTree(dictionary.root);
        else if (strcmp(input, "+inserisci_inizio") == 0) {
            inserisci_inizio(in_at, min_occ, occ, not_in_at);
        } else {
            assert(strlen(input) == k);
            if (_strcmp(input, ref_word) == 0) {
                printf("ok\n");
                reset(dictionary.root);
                return;
            } else if (search(dictionary.root, input) != dictionary.nil) {
                char _min_occ[ALPHABET_LENGTH] = {0};
                char res[k];
                char used[k];

                int new_occ = 0;
                int to_filter_occ[k][2];
                int to_filter_in_at[k][2];
                int new_in_at = 0;
                int to_filter_min_occ[k][2];
                int new_min_occ = 0;
                int to_filter_not_in_at[k][2];
                int new_not_in_at = 0;

                for (int i = 0; i < k; i++) {
                    res[i] = 0;
                    used[i] = 0;
                }
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
                        _min_occ[hash(input[j])]++;
                    }
                }

                for (int j = 0; j < k; j++) {
                    if (res[j] != '+') {
                        int found = 0;
                        const int hashed = hash(input[j]);

                        for (int i = 0; i < k; i++) {
                            if (ref_word[i] == input[j] && used[i] != 1) {
                                res[j] = '|';

                                used[i] = 1;
                                found = 1;
                                _min_occ[hashed]++;

                                if (!not_in_at[j][hashed]) {
                                    not_in_at[j][hashed] = 1;
                                    to_filter_not_in_at[new_not_in_at][0] = input[j];
                                    to_filter_not_in_at[new_not_in_at][1] = j;
                                    new_not_in_at++;
                                }
                                break;
                            }
                        }
                        if (_min_occ[hashed] > min_occ[hashed]) {
                            min_occ[hashed] = _min_occ[hashed];
                            to_filter_min_occ[new_min_occ][0] = input[j];
                            to_filter_min_occ[new_min_occ][1] = min_occ[hashed];
                            new_min_occ++;
                        }
                        if (!found) {
                            res[j] = '/';
                            //not_in[hash(input[j])] = 1;

                            if (occ[hashed] == -1) {
                                occ[hashed] = min_occ[hashed];
                                to_filter_occ[new_occ][0] = input[j];
                                to_filter_occ[new_occ][1] = occ[hashed];
                                new_occ++;
                            }

                            if (!not_in_at[j][hashed]) {
                                not_in_at[j][hashed] = 1;
                                if (occ[hashed] != 0) {
                                    to_filter_not_in_at[new_not_in_at][0] = input[j];
                                    to_filter_not_in_at[new_not_in_at][1] = j;
                                    new_not_in_at++;
                                }
                            }
                        }
                    }
                }

                for (int h = 0; h < new_occ; h++) {
                    for (int h2 = h + 1; h2 < new_occ; h2++) {
                        if (to_filter_occ[h][0] == to_filter_occ[h2][0]) {
                            if (to_filter_occ[h][1] > to_filter_occ[h2][1]) {
                                to_filter_occ[h2][0] = to_filter_occ[new_occ - 1][0];
                                to_filter_occ[h2][1] = to_filter_occ[new_occ - 1][1];
                            } else {
                                to_filter_occ[h][0] = to_filter_occ[new_occ - 1][0];
                                to_filter_occ[h][1] = to_filter_occ[new_occ - 1][1];
                            }
                            new_occ--;
                            h2--;
                        }
                    }
                }
                for (int h = 0; h < new_min_occ; h++) {
                    for (int h2 = h + 1; h2 < new_min_occ; h2++) {
                        if (to_filter_min_occ[h][0] == to_filter_min_occ[h2][0]) {
                            if (to_filter_min_occ[h][1] > to_filter_min_occ[h2][1]) {
                                to_filter_min_occ[h2][0] = to_filter_min_occ[new_min_occ - 1][0];
                                to_filter_min_occ[h2][1] = to_filter_min_occ[new_min_occ - 1][1];
                            } else {
                                to_filter_min_occ[h][0] = to_filter_min_occ[new_min_occ - 1][0];
                                to_filter_min_occ[h][1] = to_filter_min_occ[new_min_occ - 1][1];
                            }
                            new_min_occ--;
                            h2--;
                        }
                    }
                }

                if (new_occ > 0 || new_not_in_at > 0 || new_min_occ > 0 || new_in_at > 0) {
                    apply_filters(to_filter_occ, new_occ, to_filter_in_at,
                                  new_in_at,
                                  to_filter_min_occ, new_min_occ, to_filter_not_in_at, new_not_in_at);
                    //num_filtered_nodes -= delete_in(&dictionary, to_delete);
                }

                for (int j = 0; j < k; j++) {
                    printf("%c", res[j]);
                }

                printf("\n%d\n", num_filtered_nodes);
                n--;
                //printf("CICLI: %d\n", ccc);
            } else {
                printf("not_exists\n");
            }
        }
    }
    printf("ko\n");
    reset(dictionary.root);
}

int main() {
    dictionary.nil = malloc(sizeof(node_t));
    dictionary.nil->word = NULL;
    dictionary.nil->color = BLACK;
    dictionary.root = dictionary.nil;
    dictionary.head = NULL;
#ifdef EVAL
    setvbuf(stdout, print_buffer, _IOFBF, sizeof(print_buffer));
#else
    setvbuf(stdout, NULL, _IONBF, 0);
#endif
    if (scanf("%d", &k)) {
        malloc_word_size = k * NUM_NODES_PER_MALLOC_INIT + 1;
        malloc_node_size = node_size * NUM_NODES_PER_MALLOC_INIT;
        if (k >= 64) read_length = k + 1;
        node_t *x;
        char *word_buffer = malloc(malloc_word_size);
        void *node_buffer = malloc(malloc_node_size);
        int num_nodes = 0;
        char adding_words = 1;
        char read[read_length];
        while (scanf("%s", read) > 0) {
            if (strcmp(read, "+nuova_partita") == 0) {
                adding_words = 0;
                nuova_partita();
            } else if (strcmp(read, "+inserisci_inizio") == 0)
                adding_words = 1;
            else if (adding_words && strcmp(read, "+inserisci_fine") == 0)
                adding_words = 0;
            else if (adding_words && strcmp(read, "+stampa_filtrate") != 0) {
                assert(strlen(read) == k);
                if (num_nodes >= NUM_NODES_PER_MALLOC_INIT) {
                    node_buffer = malloc(malloc_node_size);
                    assert(node_buffer);
                    word_buffer = malloc(malloc_word_size);
                    assert(word_buffer);
#ifndef EVAL
                    word_buffer[0] = '\0';
#endif
                    num_nodes = 0;
                }
                char *word = word_buffer + num_nodes * k;
                strncpy(word, read, k);
#ifndef EVAL
                word_buffer[num_nodes * k + k] = '\0';
#endif
                x = node_buffer + num_nodes * node_size;
                x->word = word;
                x->deleted = 0;
                insert(x);
                num_filtered_nodes++;
                node_t *temp = dictionary.head;
                dictionary.head = x;
                x->prev = NULL;
                x->next = temp;
                if (temp)temp->prev = x;
                num_nodes++;
            }
        }
    }
    return 0;
}
