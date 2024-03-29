#ifdef EVAL
#define NDEBUG
#endif

#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <assert.h>
#include <stdbool.h>
#include <stdint.h>

#define RED 0
#define BLACK 1
#define ALPHABET_LENGTH 64
#define NUM_NODES_PER_MALLOC 10000
#define NUM_EL_FREE_LIST 100


#ifdef EVAL
char print_buffer[10000];
#endif


typedef struct node {
    struct node *parent; //Contiene due booleani nei 2 bit meno significativi
    struct node *left;
    struct node *right;
    struct node *next;
    char *word;
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
int read_length = 64;
void *(*to_free_list)[2];
int num_el_to_free = 0;

static inline int _strcmp(const char s1[k], const char s2[k]) {
    for (int i = 0; i < k; i++) {
        if (s1[i] != s2[i])
            return s1[i] - s2[i];
    }
    return 0;
    //return strncmp(s1, s2, k);
}

static inline bool is_deleted(const node_t *const node) {
    return (uintptr_t) node->parent & 1;
}

static inline void set_deleted(node_t *const node, const bool deleted) {
    node->parent = (node_t *) (((uintptr_t) node->parent & 0xFFFFFFFFFFFFFFFE) | deleted);
}

static inline unsigned char get_color(const node_t *const node) {
    return ((uintptr_t) node->parent & 2) >> 1;
}

static inline void set_color(node_t *const node, const unsigned char color) {
    node->parent = (node_t *) (((uintptr_t) node->parent & 0xFFFFFFFFFFFFFFFD) | (color << 1));
}

static inline node_t *get_parent(const node_t *const node) {
    return (node_t *) ((uintptr_t) node->parent & 0xFFFFFFFFFFFFFFFC);
}

static inline void set_parent(node_t *const node, const node_t *const parent) {
    node->parent = (node_t *) (((uintptr_t) node->parent & 0x3) | ((uintptr_t) parent & 0xFFFFFFFFFFFFFFFC));
}

void left_rotate(node_t *const x) {
    node_t *y = x->right;
    x->right = y->left; //il sottoalbero sinistro di y diventa quello destro di x
    if (y->left != dictionary.nil)
        set_parent(y->left, x);
    set_parent(y, get_parent(x)); //attacca il padre di x a y
    if (get_parent(x) == dictionary.nil)
        dictionary.root = y;
    else if (x == get_parent(x)->left)
        get_parent(x)->left = y;
    else
        get_parent(x)->right = y;
    y->left = x; //mette x a sinistra di y
    set_parent(x, y);
}

void right_rotate(node_t *const x) {
    node_t *y = x->left;
    x->left = y->right; //il sottoalbero sinistro di y diventa quello destro di x
    if (y->right != dictionary.nil)
        set_parent(y->right, x);
    set_parent(y, get_parent(x)); //attacca il padre di x a y
    if (get_parent(x) == dictionary.nil)
        dictionary.root = y;
    else if (x == get_parent(x)->right)
        get_parent(x)->right = y;
    else
        get_parent(x)->left = y;
    y->right = x; //mette x a sinistra di y
    set_parent(x, y);
}

void insert_fixup(node_t *z) {
    if (z == dictionary.root)
        set_color(dictionary.root, BLACK);
    else {
        node_t *x = get_parent(z); // x e' il padre di z
        if (get_color(x) == RED) {
            if (x == get_parent(x)->left) {// se x e' figlio sin.ì
                node_t *y = get_parent(x)->right; // y e' fratello di x
                if (/*y && */get_color(y) == RED) {
                    set_color(x, BLACK); // Caso 1
                    set_color(y, BLACK); // Caso 1
                    set_color(get_parent(x), RED); // Caso 1
                    insert_fixup(get_parent(x)); // Caso 1
                } else {
                    if (z == x->right) {
                        z = x; // Caso 2
                        left_rotate(z); // Caso 2
                        x = get_parent(z); // Caso 2
                    }
                    set_color(x, BLACK); // Caso 3
                    set_color(get_parent(x), RED); // Caso 3
                    right_rotate(get_parent(x)); // Caso 3
                }
            } else {//(come 6 - 18, scambiando “right”↔“left”)
                assert(x == get_parent(x)->right);
                node_t *y = get_parent(x)->left; // y e' fratello di x
                if (/*y && */get_color(y) == RED) {
                    set_color(x, BLACK); // Caso 1
                    set_color(y, BLACK); // Caso 1
                    set_color(get_parent(x), RED); // Caso 1
                    insert_fixup(get_parent(x)); // Caso 1
                } else {
                    if (z == x->left) {
                        z = x; // Caso 2
                        right_rotate(z); // Caso 2
                        x = get_parent(z); // Caso 2
                    }
                    set_color(x, BLACK); // Caso 3
                    set_color(get_parent(x), RED); // Caso 3
                    left_rotate(get_parent(x)); // Caso 3
                }
            }
        }
    }
}

void insert(node_t *const z) {
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
    set_parent(z, y);
    if (y == dictionary.nil)
        dictionary.root = z; //l'albero T e' vuoto
    else if (_strcmp(z_word, y->word) < 0)
        y->left = z;
    else
        y->right = z;
    z->left = dictionary.nil;
    z->right = dictionary.nil;
    set_color(z, RED);
    insert_fixup(z);
}

static inline void print(const char s[k]) {
    for (int i = 0; i < k; i++)
        putchar_unlocked(s[i]);
    putchar_unlocked('\n');
}

void print_tree(const node_t *const x) {
    if (x->left != dictionary.nil)print_tree(x->left);
    if (!is_deleted(x))print(x->word);
    if (x->right != dictionary.nil)print_tree(x->right);
}

bool is_present(const node_t *const x, const char word[k]) {
    if (x == dictionary.nil)
        return false;
    else if (_strcmp(word, x->word) == 0)
        return true;

    if (_strcmp(word, x->word) < 0)
        return is_present(x->left, word);
    else return is_present(x->right, word);
}

static inline unsigned char hash(const char c) {
    if (c == '_')return 0;
    if (c == '-')return 1;
    if (c >= '0' && c <= '9') return c - 46;    //2-11
    if (/*c >= 'A' &&*/ c <= 'Z') return c - 53;    //12-37
    if (/*c >= 'a' &&*/ c <= 'z') return c - 59;    //38-63
    return -1;
}

static inline char dehash(const unsigned char i) {
    if (i == 0)return '_';
    if (i == 1)return '-';
    if (i >= 2 && i <= 11) return (char) (i + 46);    //2-11
    if (/*i >= 12 &&*/ i <= 37) return (char) (i + 53);    //12-37
    if (/*i >= 38 && */i <= 63) return (char) (i + 59);    //38-63
    return -1;
}

bool check_filters(const char word[k],
                   const int in_at[k][2], const int num_in_at,
                   const int min_occ[ALPHABET_LENGTH][2], const int num_min_occ,
                   const int occ[ALPHABET_LENGTH][2], const int num_occ,
                   const int not_in_at[k * ALPHABET_LENGTH][2], const int num_not_in_at) {

    for (int i = 0; i < k; i++) {
        if (i < num_in_at && in_at[i][0] != word[in_at[i][1]]) {
            return false;
        }
        for (int j = 0; i * ALPHABET_LENGTH + j < num_not_in_at && j < ALPHABET_LENGTH; j++) {
            if (not_in_at[i * ALPHABET_LENGTH + j][0] == word[not_in_at[i * ALPHABET_LENGTH + j][1]]) {
                return false;
            }
        }
    }
    for (int i = 0; i < ALPHABET_LENGTH; i++) {
        if (i < num_occ) {
            const char c = occ[i][0];
            int count = 0;
            for (int j = 0; j < k; j++) {
                if (word[j] == c)
                    count++;
            }
            if (count != occ[i][1])
                return false;
        }
        if (i < num_min_occ) {
            const char c = min_occ[i][0];
            int count = 0;
            for (int j = 0; j < k; j++) {
                if (word[j] == c)
                    count++;
            }
            if (count < min_occ[i][1])
                return false;
        }
    }
    return true;
}

static inline void add_to_free_list(node_t *const node_buffer, char *const word_buffer) {
    if (num_el_to_free % NUM_EL_FREE_LIST == 0) {
        to_free_list = realloc(to_free_list, num_el_to_free * sizeof(*to_free_list) +
                                             NUM_EL_FREE_LIST * sizeof(*to_free_list));
#ifndef EVAL
        fprintf(stderr, "REALLOC: %d\n", num_el_to_free);
#endif
    }
    to_free_list[num_el_to_free][0] = node_buffer;
    to_free_list[num_el_to_free][1] = word_buffer;
    num_el_to_free++;
}

static inline void add_node(const char read[k], const bool filtered) {
    static char *word_buffer = NULL;
    static node_t *node_buffer = NULL;
    static int num_nodes = 0;

    if (!node_buffer || num_nodes >= NUM_NODES_PER_MALLOC) {
        node_buffer = malloc(malloc_node_size);
        assert(node_buffer);
        word_buffer = malloc(malloc_word_size);
        assert(word_buffer);
#ifndef EVAL
        word_buffer[0] = '\0';
#endif
        num_nodes = 0;
        add_to_free_list(node_buffer, word_buffer);
    }
    char *word = word_buffer + num_nodes * k;
    strncpy(word, read, k);
#ifndef EVAL
    word_buffer[num_nodes * k + k] = '\0';
#endif
    node_t *new_node = node_buffer + num_nodes;
    new_node->word = word;
    insert(new_node);
    if (filtered) {
        set_deleted(new_node, false);
        num_filtered_nodes++;
        node_t *temp = dictionary.head;
        dictionary.head = new_node;
        new_node->next = temp;
    } else {
        set_deleted(new_node, true);
    }
    num_nodes++;
}

void inserisci_inizio(const char in_at[k], const int min_occ[ALPHABET_LENGTH], const int occ[ALPHABET_LENGTH],
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
            const char c = dehash(i);
            _occ[num_occ][0] = c;
            _occ[num_occ][1] = occ[i];
            num_occ++;
        } else if (min_occ[i] > 0) {
            const char c = dehash(i);
            _min_occ[num_min_occ][0] = c;
            _min_occ[num_min_occ][1] = min_occ[i];
            num_min_occ++;
        }
    }

    char read[read_length];
    while (scanf("%s", read) > 0) {
        if (strcmp(read, "+inserisci_fine") == 0)
            return;
        assert (strlen(read) == k);
        add_node(read, check_filters(read,
                                     _in_at, num_in_at,
                                     _min_occ, num_min_occ,
                                     _occ, num_occ,
                                     _not_in_at, num_not_in_at));
    }
}

void apply_filters(
        const int to_filter_occ[][2], const int new_occ,
        const int to_filter_in_at[][2], const int new_in_at,
        const int to_filter_min_occ[][2], const int new_min_occ,
        const int to_filter_not_in_at[][2], const int new_not_in_at) {
    node_t *index = dictionary.head;
    node_t *prev = NULL, *next;
    const char *word;
    while (index) {
        assert(!is_deleted(index));
        bool deleted = false;
        next = index->next;
        word = index->word;
        for (int i = 0; i < k; i++) {

            /*Filtro per lettere già trovate
             * to_filter_in_at[][0] = lettera trovata
             * to_filter_in_at[][1] = posizione
             */
            if (i < new_in_at) {
                if (word[to_filter_in_at[i][1]] != to_filter_in_at[i][0]) {
                    set_deleted(index, true);
                    num_filtered_nodes--;
                    if (!prev) {
                        dictionary.head = dictionary.head->next;
                    } else {
                        prev->next = index->next;
                    }
                    deleted = true;
                    break;
                }
            }

            /*Filtro per lettere not presenti in una posizione
             * to_filter_not_in_at[][0] = lettera
             * to_filter_not_in_at[][1] = posizione
             */
            if (i < new_not_in_at) {
                if (word[to_filter_not_in_at[i][1]] == to_filter_not_in_at[i][0]) {
                    set_deleted(index, true);
                    num_filtered_nodes--;
                    if (!prev) {
                        dictionary.head = dictionary.head->next;
                    } else {
                        prev->next = index->next;
                    }
                    deleted = true;
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
                    set_deleted(index, true);
                    num_filtered_nodes--;
                    if (!prev) {
                        dictionary.head = dictionary.head->next;
                    } else {
                        prev->next = index->next;
                    }
                    deleted = true;
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
                    set_deleted(index, true);
                    num_filtered_nodes--;
                    if (!prev) {
                        dictionary.head = dictionary.head->next;
                    } else {
                        prev->next = index->next;
                    }
                    deleted = true;
                    break;
                }
            }
        }
        if (!deleted)
            prev = index;
        index = next;
    }
}

void reset(node_t *const x) {
    if (x->left != dictionary.nil) reset(x->left);
    if (x->right != dictionary.nil) reset(x->right);

    if (is_deleted(x)) {
        set_deleted(x, false);
        num_filtered_nodes++;
        node_t *temp = dictionary.head;
        dictionary.head = x;
        x->next = temp;
    }
}

int cmp(const void *v1, const void *v2) {
    if (((int *) v1)[0] != ((int *) v2)[0])return ((int *) v1)[0] - ((int *) v2)[0];
    return ((int *) v1)[1] - ((int *) v2)[1];
}

void nuova_partita() {
    char ref_word[k + 1];
    int min_occ[ALPHABET_LENGTH] = {0};
    int occ[ALPHABET_LENGTH];
    char in_at[k];
    char not_in_at[k][ALPHABET_LENGTH];

    memset(in_at, -1, k);
    memset(occ, -1, ALPHABET_LENGTH * sizeof(int));
    memset(not_in_at, 0, k * ALPHABET_LENGTH);

    if (!scanf("%s", ref_word)) return;
    assert(is_present(dictionary.root, ref_word));
    int n;
    if (!scanf("%d", &n)) return;
    while (n > 0) {
        char input[read_length];
        if (scanf("%s", input) < 0)break;
        if (strcmp(input, "+stampa_filtrate") == 0 && num_filtered_nodes > 0) {
            assert(dictionary.root != dictionary.nil);
            print_tree(dictionary.root);
        } else if (strcmp(input, "+inserisci_inizio") == 0) {
            inserisci_inizio(in_at, min_occ, occ, not_in_at);
        } else {
            assert(strlen(input) == k);
            if (_strcmp(input, ref_word) == 0) {
                printf("ok\n");
                if (dictionary.root != dictionary.nil)
                    reset(dictionary.root);
                return;
            } else if (is_present(dictionary.root, input)) {
                int _min_occ[ALPHABET_LENGTH] = {0};
                char res[k];
                bool used[k];

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
                    used[i] = false;
                }
                for (int j = 0; j < k; j++) {
                    if (ref_word[j] == input[j]) {
                        res[j] = '+';
                        used[j] = true;

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
                        const unsigned char hashed = hash(input[j]);

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

                qsort(to_filter_min_occ, new_min_occ, sizeof(int) * 2, cmp);
                for (int h = 0; h < new_min_occ - 1; h++) {
                    if (to_filter_min_occ[h][0] == to_filter_min_occ[h + 1][0]) {
                        for (int i = h; i < new_min_occ - 1; i++) {
                            to_filter_min_occ[i][0] = to_filter_min_occ[i + 1][0];
                            to_filter_min_occ[i][1] = to_filter_min_occ[i + 1][1];
                        }
                        new_min_occ--;
                        h--;
                    }
                }

                if (new_occ > 0 || new_not_in_at > 0 || new_min_occ > 0 || new_in_at > 0) {
                    apply_filters(to_filter_occ, new_occ,
                                  to_filter_in_at, new_in_at,
                                  to_filter_min_occ, new_min_occ,
                                  to_filter_not_in_at, new_not_in_at);
                }

                for (int j = 0; j < k; j++) {
                    putchar_unlocked(res[j]);
                }

                printf("\n%d\n", num_filtered_nodes);
                n--;
            } else {
                printf("not_exists\n");
            }
        }
    }
    printf("ko\n");
    if (dictionary.root != dictionary.nil)
        reset(dictionary.root);
}

void free_tree() {
    for (int i = 0; i < num_el_to_free; i++) {
        free(to_free_list[i][0]);
        free(to_free_list[i][1]);
    }
}

int main() {
    dictionary.nil = malloc(sizeof(node_t));
    dictionary.nil->word = NULL;
    dictionary.nil->left = NULL;
    dictionary.nil->right = NULL;
    dictionary.nil->parent = NULL;
    set_color(dictionary.nil, BLACK);
    set_deleted(dictionary.nil, false);
    dictionary.root = dictionary.nil;
    dictionary.head = NULL;
#ifdef EVAL
    setvbuf(stdout, print_buffer, _IOFBF, sizeof(print_buffer));
#else
    setvbuf(stdout, NULL, _IONBF, 0);
#endif
    if (scanf("%d", &k)) {
#ifdef EVAL
        malloc_word_size = k * NUM_NODES_PER_MALLOC;
#else
        malloc_word_size = k * NUM_NODES_PER_MALLOC + 1;
#endif
        malloc_node_size = sizeof(node_t) * NUM_NODES_PER_MALLOC;
        if (k >= 64) read_length = k + 1;
        bool adding_words = true;
        char read[read_length];
        while (scanf("%s", read) > 0) {
            if (strcmp(read, "+nuova_partita") == 0) {
                adding_words = false;
                nuova_partita();
            } else if (strcmp(read, "+inserisci_inizio") == 0)
                adding_words = true;
            else if (adding_words && strcmp(read, "+inserisci_fine") == 0)
                adding_words = false;
            else if (adding_words && strcmp(read, "+stampa_filtrate") != 0) {
                assert(strlen(read) == k);
                add_node(read, true);
            }
        }
        free_tree();
        free(to_free_list);
    }
    free(dictionary.nil);
    return 0;
}
