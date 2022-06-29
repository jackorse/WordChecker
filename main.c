#include <stdio.h>
#include <string.h>
#include <stdlib.h>

#define RED 0
#define BLACK 1
#define ALPHABET_LENGTH 64

typedef struct node {
    struct node *parent;
    struct node *left;
    struct node *right;
    char *word;
    char color;
} node_t;

typedef struct tree {
    node_t *root;
    node_t *nil;
} RB_tree;

int k;
RB_tree dictionary;

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
        if (strcmp(z->word, x->word) < 0)
            x = x->left;
        else
            x = x->right;
    }
    z->parent = y;
    if (y == tree->nil)
        tree->root = z; //l'albero T e' vuoto
    else if (strcmp(z->word, y->word) < 0)
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
        printf("%s\n", x->word);
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
    if (!x->word || strcmp(word, x->word) == 0)
        return x;
    if (strcmp(word, x->word) < 0)
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
            w = x->parent->right;
        }// Caso 1
        char lcolor = BLACK, rcolor = BLACK;
        if (w->left) lcolor = w->left->color;
        if (w->right) rcolor = w->right->color;
        if (lcolor == BLACK && rcolor == BLACK) {
            w->color = RED; // Caso 2
            delete_fixup(tree, x->parent); // Caso 2
        } else {
            if (rcolor == BLACK) {
                w->left->color = BLACK; // Caso 3
                w->color = RED; // Caso 3
                right_rotate(tree, w); // Caso 3
                w = x->parent->right; // Caso 3
            }
            w->color = x->parent->color; // Caso 4
            x->parent->color = BLACK; // Caso 4
            if (w->right)w->right->color = BLACK; // Caso 4
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
        char lcolor = BLACK, rcolor = BLACK;
        if (w->left) lcolor = w->left->color;
        if (w->right) rcolor = w->right->color;
        if (rcolor == BLACK && lcolor == BLACK) {
            w->color = RED; // Caso 2
            delete_fixup(tree, x->parent); // Caso 2
        } else {
            if (lcolor == BLACK) {
                w->right->color = BLACK; // Caso 3
                w->color = RED; // Caso 3
                left_rotate(tree, w); // Caso 3
                w = x->parent->left; // Caso 3
            }
            w->color = x->parent->color; // Caso 4
            x->parent->color = BLACK; // Caso 4
            if (w->left)w->left->color = BLACK; // Caso 4
            right_rotate(tree, x->parent); // Caso 4
        }
    }
}

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
    /*if (x) */x->parent = y->parent; // x potrebbe essere T.nil;
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
}

node_t *copy(const RB_tree *tree, const node_t *x, node_t *p) {
    if (x != dictionary.nil) {
        node_t *node = malloc(sizeof(node_t));
        node->color = x->color;
        node->parent = p;
        node->word = malloc(sizeof(char[k]) + 1);
        strcpy(node->word, x->word);
        node->left = copy(tree, x->left, node);
        node->right = copy(tree, x->right, node);
        return node;
    }
    return tree->nil;
}

typedef struct list_node {
    char *word;
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


static inline void add_to_del_list(list *toDelete_head, list *toDelete_index, char word[k + 1]) {
    while (*toDelete_index && (*toDelete_index)->next && strcmp(word, (*toDelete_index)->next->word) > 0) {
        /*if (streq(index->word, x->word, k)) {
            found = 1;
            break;
        }*/
        *toDelete_index = (*toDelete_index)->next;
    }
    //if (!found) {
    //printf("Adding %s", x->word);
    char head_changed = 0;
    if (*toDelete_index == *toDelete_head) head_changed = 1;
    if (!*toDelete_index) {
        *toDelete_index = malloc(sizeof(list_node_t));
        (*toDelete_index)->next = NULL;
        (*toDelete_index)->word = malloc(sizeof(char[k]) + 1);
        strcpy((*toDelete_index)->word, word);
    } else if (!(*toDelete_index)->next && strcmp((*toDelete_index)->word, word) > 0) {
        list_node_t *temp = *toDelete_index;
        *toDelete_index = malloc(sizeof(list_node_t));
        (*toDelete_index)->next = temp;
        (*toDelete_index)->word = malloc(sizeof(char[k]) + 1);
        strcpy((*toDelete_index)->word, word);
    } else if ((!(*toDelete_index)->next && strcmp((*toDelete_index)->word, word) < 0) ||
               ((*toDelete_index)->next && strcmp((*toDelete_index)->next->word, word) != 0)) {
        list_node_t *new = malloc(sizeof(list_node_t));
        new->next = (*toDelete_index)->next;
        new->word = malloc(sizeof(char[k]) + 1);
        strcpy(new->word, word);
        (*toDelete_index)->next = new;
    }
    if (head_changed) *toDelete_head = *toDelete_index;
}

void find_without_at(const RB_tree *tree, node_t *x, char c, int pos, list *toDelete_head, list *toDelete_index) {
    if (x == tree->nil) return;
    find_without_at(tree, x->left, c, pos, toDelete_head, toDelete_index);
    if (x->word[pos] != c) {
        //list_node_t *index = *toDelete;
        //char found = 0;
        /*while (index) {
            if (streq(index->word, x->word, k)) {
                found = 1;
                break;
            }
            index = index->next;
        }*/
        //if (!found) {
        /*list_node_t *temp = *toDelete;
        *toDelete = malloc(sizeof(list_node_t));
        (*toDelete)->next = temp;
        (*toDelete)->word = malloc(sizeof(char[k]) + 1);
        strcpy((*toDelete)->word, x->word);*/
        //}
        /*if (!**toDelete_head) {
            **toDelete_head = malloc(sizeof(list_node_t));
            (**toDelete_head)->next = NULL;
            (**toDelete_head)->word = malloc(sizeof(char[k]) + 1);
            strcpy((**toDelete_head)->word, x->word);
            toDelete_index = toDelete_head;
        }*/

        add_to_del_list(toDelete_head, toDelete_index, x->word);
    }
    find_without_at(tree, x->right, c, pos, toDelete_head, toDelete_index);
}

void find_with_at(const RB_tree *tree, node_t *x, char c, int pos, list *toDelete_head, list *toDelete_index) {
    if (x == tree->nil) return;
    find_with_at(tree, x->left, c, pos, toDelete_head, toDelete_index);
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


        add_to_del_list(toDelete_head, toDelete_index, x->word);
    }
    find_with_at(tree, x->right, c, pos, toDelete_head, toDelete_index);
}

void find_with(const RB_tree *tree, node_t *x, char c, int occ, list *toDelete_head, list *toDelete_index) {
    if (x == tree->nil) return;
    find_with(tree, x->left, c, occ, toDelete_head, toDelete_index);
    int count = 0;
    for (int i = 0; i < k; i++) {
        if (x->word[i] == c)
            count++;
    }
    if (occ != count) {
        //list_node_t *index = *toDelete;
        //char found = 0;
        /*while (index) {
            if (strcmp(index->word, x->word) == 0) {
                found = 1;
                break;
            }
            index = index->next;
        }*/
        //if (!found) {
        /*list_node_t *temp = *toDelete;
        *toDelete = malloc(sizeof(list_node_t));
        (*toDelete)->next = temp;
        (*toDelete)->word = malloc(sizeof(char[k]) + 1);
        strcpy((*toDelete)->word, x->word);*/
        //}

        add_to_del_list(toDelete_head, toDelete_index, x->word);
    }
    find_with(tree, x->right, c, occ, toDelete_head, toDelete_index);
}

void find_with_min_occ(const RB_tree *tree, node_t *x, char c, int min, list *toDelete_head, list *toDelete_index) {
    if (x == tree->nil) return;
    find_with_min_occ(tree, x->left, c, min, toDelete_head, toDelete_index);
    int count = 0;
    for (int i = 0; i < k; i++) {
        if (x->word[i] == c)
            count++;
    }
    if (count < min) {
        //char found = 0;

        add_to_del_list(toDelete_head, toDelete_index, x->word);
    }
    find_with_min_occ(tree, x->right, c, min, toDelete_head, toDelete_index);
}


void free_list(list l) {
    list_node_t *temp;
    while (l) {
        temp = l;
        l = temp->next;
        free(temp->word);
        free(temp);
    }
}

void delete_in(RB_tree *tree, list *toDelete) {
    list_node_t *index = *toDelete;
    while (index) {
        node_t *t = search(tree->root, index->word);
        if (t != tree->nil) {
            t = delete(tree, t);
            free(t->word);
            free(t);
        }
        index = index->next;
    }
    free_list(*toDelete);
    *toDelete = NULL;
}

void inserisci_inizio(RB_tree *dict) {
    node_t *new_dict, *new_filtered;
    while (1) {
        char read[256];
        if (scanf("%s", read) < 0)break;
        if (strcmp(read, "+inserisci_fine") == 0)
            return;
        if (strlen(read) == k) {
            char *new_dict_word = malloc(sizeof(char[k + 1]));
            char *new_filtered_word = malloc(sizeof(char[k + 1]));
            strcpy(new_dict_word, read);
            strcpy(new_filtered_word, read);
            new_dict = malloc(sizeof(node_t));
            new_filtered = malloc(sizeof(node_t));
            new_dict->word = new_dict_word;
            new_dict->left = dictionary.nil;
            new_dict->right = dictionary.nil;
            new_filtered->word = new_filtered_word;
            new_filtered->left = dict->nil;
            new_filtered->right = dict->nil;
            insert(&dictionary, new_dict);
            insert(dict, new_filtered);
        }
    }
}

static inline int hash(char c) {
    if (c == '_')return 0;
    if (c == '-')return 1;
    if (c >= '0' && c <= '9') return c - 46;    //2-11
    if (c >= 'A' && c <= 'Z') return c - 53;    //12-37
    if (c >= 'a' && c <= 'z') return c - 59;    //38-63
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

void apply_filters(RB_tree *filtered_tree, const char *in_at, /*const char not_in[ALPHABET_LENGTH],*/
                   const char min_occ[ALPHABET_LENGTH], const char occ[ALPHABET_LENGTH],
                   const char not_in_at[k][ALPHABET_LENGTH]) {
    if (!filtered_tree->root) {
        printf("filtered_tree null!!!");
        return;
    }

    list head = NULL;
    list index = head;

    for (int i = 0; i < k; i++) {
        if (in_at[i] >= 0) {
            char c = in_at[i];
            find_without_at(filtered_tree, filtered_tree->root, c, i, &head, &index);
            index = head;
        }

        for (int j = 0; j < ALPHABET_LENGTH; j++) {
            if (not_in_at[i][j]) {
                char c = dehash(j);
                find_with_at(filtered_tree, filtered_tree->root, c, i, &head, &index);
                index = head;
            }
        }
    }
    for (int i = 0; i < ALPHABET_LENGTH; i++) {
        if (occ[i] >= 0) {
            char c = dehash(i);
            find_with(filtered_tree, filtered_tree->root, c, occ[i], &head, &index);
            index = head;
        } else if (min_occ[i] > 0) {
            char c = dehash(i);
            find_with_min_occ(filtered_tree, filtered_tree->root, c, min_occ[i], &head, &index);
            index = head;
        }
    }
    delete_in(filtered_tree, &head);
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
    int c = 1;
    c += count(tree, x->left);
    c += count(tree, x->right);
    return c;
}

void destroy(RB_tree *tree, node_t *x) {
    if (x->left != tree->nil) destroy(tree, x->left);
    if (x->right != tree->nil) destroy(tree, x->right);

    if (x == tree->root) {
        //free(tree->nil->word);
        free(tree->nil);
    }
    free(x->word);
    free(x);
}

void nuova_partita() {
    char ref_word[k + 1];
    //char not_in[ALPHABET_LENGTH] = {0};
    char min_occ[ALPHABET_LENGTH] = {0};
    char occ[ALPHABET_LENGTH];
    char in_at[k];
    char not_in_at[k][ALPHABET_LENGTH];
    for (int i = 0; i < k; i++) in_at[i] = -1;
    for (int i = 0; i < ALPHABET_LENGTH; i++) occ[i] = -1;
    for (int i = 0; i < k; i++)
        for (int j = 0; j < ALPHABET_LENGTH; j++)
            not_in_at[i][j] = 0;

    if (!scanf("%s", ref_word)) return;
    int n;
    if (!scanf("%d", &n)) return;
    RB_tree filtered_tree;
    filtered_tree.nil = malloc(sizeof(node_t));
    filtered_tree.nil->word = NULL;
    filtered_tree.nil->color = BLACK;
    filtered_tree.root = filtered_tree.nil;
    filtered_tree.root = copy(&filtered_tree, dictionary.root, filtered_tree.nil);
    //printTree(filtered_tree);
    while (n > 0) {
        char input[256];
        if (scanf("%s", input) < 0)break;
        if (strcmp(input, "+stampa_filtrate") == 0)
            printTree(filtered_tree.root);
        else if (strcmp(input, "+inserisci_inizio") == 0) {
            inserisci_inizio(&filtered_tree);
            apply_filters(&filtered_tree, in_at,/* not_in, */min_occ, occ, not_in_at);
        } else if (strlen(input) == k) {
            if (strcmp(input, ref_word) == 0) {
                printf("ok\n");
                destroy(&filtered_tree, filtered_tree.root);
                return;
            } else if (search(dictionary.root, input) != dictionary.nil) {      //O(logn)
                char _min_occ[ALPHABET_LENGTH] = {0};
                char res[k];
                char used[k];
                for (int i = 0; i < k; i++) {
                    res[i] = 0;
                    used[i] = 0;
                }
                //list toDelete = NULL;
                for (int j = 0; j < k; j++) {                                            //O(k)
                    if (ref_word[j] == input[j]) {
                        res[j] = '+';
                        used[j] = 1;
                        in_at[j] = input[j];
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
                //apply_filter_in_at(&filtered_tree, in_at, k);

                for (int j = 0; j < k; j++) {                                               //O(k^2)
                    if (res[j] != '+') {
                        int found = 0;

                        for (int i = 0; i < k; i++) {
                            if (ref_word[i] == input[j] && used[i] != 1) {
                                res[j] = '|';

                                used[i] = 1;
                                found = 1;
                                _min_occ[hash(input[j])]++;
                                not_in_at[j][hash(input[j])] = 1;
                                break;
                            }
                        }
                        if (_min_occ[hash(input[j])] > min_occ[hash(input[j])])
                            min_occ[hash(input[j])] = _min_occ[hash(input[j])];
                        if (!found) {
                            res[j] = '/';
                            //not_in[hash(input[j])] = 1;
                            not_in_at[j][hash(input[j])] = 1;
                            occ[hash(input[j])] = min_occ[hash(input[j])];
                            //find_with(filtered_tree, input[j], &toDelete);
                            //delete_in(&filtered_tree, &toDelete);
                        }
                    }
                }
                //apply_filter_not_in(&filtered_tree, not_in);
                //apply_filter_min_occ(&filtered_tree, min_occ,k);
                apply_filters(&filtered_tree, in_at,/* not_in, */min_occ, occ, not_in_at);

                for (int j = 0; j < k; j++) {
                    printf("%c", res[j]);
                }
                printf("\n%d\n", count(&filtered_tree, filtered_tree.root));
                n--;
            } else {
                printf("not_exists\n");
            }
        }
    }
    printf("ko\n");;
    destroy(&filtered_tree, filtered_tree.root);
}

int main() {
    dictionary.nil = malloc(sizeof(node_t));
    dictionary.nil->word = NULL;
    dictionary.nil->color = BLACK;
    dictionary.root = dictionary.nil;
    node_t *x;
    setvbuf(stdout, NULL, _IONBF, 0);
    if (scanf("%d", &k)) {
        while (1) {
            char temp[256];
            if (scanf("%s", temp) < 0)break;
            if (strcmp(temp, "+nuova_partita") == 0)
                nuova_partita();
            if (strlen(temp) == k) {
                char *c = malloc(sizeof(char[k + 1]));
                strcpy(c, temp);
                x = malloc(sizeof(node_t));
                x->word = c;
                insert(&dictionary, x);
            }
        }
    }
    return 0;
}
