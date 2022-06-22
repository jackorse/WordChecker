#include <stdio.h>
#include <string.h>
#include <malloc.h>
#include <stdlib.h>

#define RED 0
#define BLACK 1

typedef struct node {
    struct node *parent;
    struct node *left;
    struct node *right;
    char *word;
    char color;
} node_t;

typedef node_t *RB_tree;

void left_rotate(RB_tree *tree, node_t *x) {
    node_t *y = x->right;
    x->right = y->left; //il sottoalbero sinistro di y diventa quello destro di x
    if (y->left != NULL)
        y->left->parent = x;
    y->parent = x->parent; //attacca il padre di x a y
    if (x->parent == NULL)
        *tree = y;
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
    if (y->right != NULL)
        y->right->parent = x;
    y->parent = x->parent; //attacca il padre di x a y
    if (x->parent == NULL)
        *tree = y;
    else if (x == x->parent->right)
        x->parent->right = y;
    else
        x->parent->left = y;
    y->right = x; //mette x a sinistra di y
    x->parent = y;
}

void insert_fixup(RB_tree *tree, node_t *z) {
    if (z == *tree)
        (*tree)->color = BLACK;
    else {
        node_t *x = z->parent; // x e' il padre di z
        if (x->color == RED) {
            if (x == x->parent->left) {// se x e' figlio sin.ì
                node_t *y = x->parent->right; // y e' fratello di x
                if (y && y->color == RED) {
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
                if (y && y->color == RED) {
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
    node_t *y = NULL; // y padre del nodo considerato
    node_t *x = *tree; // nodo considerato
    while (x != NULL) {
        y = x;
        if (strcmp(z->word, x->word) < 0)
            x = x->left;
        else
            x = x->right;
    }
    z->parent = y;
    if (y == NULL)
        *tree = z; //l'albero T e' vuoto
    else if (strcmp(z->word, y->word) < 0)
        y->left = z;
    else
        y->right = z;
    z->left = NULL;
    z->right = NULL;
    z->color = RED;
    insert_fixup(tree, z);
}

void printTree(const node_t *x) {
    if (x != NULL) {
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
    if (x == NULL || strcmp(word, x->word) == 0)
        return x;
    if (strcmp(word, x->word) < 0)
        return search(x->left, word);
    else return search(x->right, word);
}

node_t *tree_minimum(node_t *x) {
    while (x->left != NULL)
        x = x->left;
    return x;
}

node_t *tree_successor(node_t *x) {
    if (x->right != NULL)
        return tree_minimum(x->right);
    node_t *y = x->parent;
    while (y != NULL && x == y->right) {
        x = y;
        y = y->parent;
    }
    return y;
}

void delete_fixup(RB_tree *T, node_t *x) {
    if (x->color == RED || x->parent == NULL)
        x->color = BLACK; // Caso 0
    else if (x == x->parent->left) {// x e' figlio sinistro
        node_t *w = x->parent->right; // w e' fratello di x
        if (w->color == RED) {
            w->color = BLACK; // Caso 1
            x->parent->color = RED; // Caso 1
            left_rotate(T, x->parent);// Caso 1
            w = x->parent->right;
        }// Caso 1
        if (w->left->color == BLACK && w->right->color == BLACK) {
            w->color = RED; // Caso 2
            delete_fixup(T, x->parent); // Caso 2
        } else if (w->right->color == BLACK) {
            w->left->color = BLACK; // Caso 3
            w->color = RED; // Caso 3
            right_rotate(T, w); // Caso 3
            w = x->parent->right; // Caso 3
        }
        w->color = x->parent->color; // Caso 4
        x->parent->color = BLACK; // Caso 4
        w->right->color = BLACK; // Caso 4
        left_rotate(T, x->parent); // Caso 4
    } else {
        node_t *w = x->parent->left; // w e' fratello di x
        if (w->color == RED) {
            w->color = BLACK; // Caso 1
            x->parent->color = RED; // Caso 1
            right_rotate(T, x->parent);// Caso 1
            w = x->parent->left;
        }// Caso 1
        if (w->right->color == BLACK && w->left->color == BLACK) {
            w->color = RED; // Caso 2
            delete_fixup(T, x->parent); // Caso 2
        } else if (w->left->color == BLACK) {
            w->right->color = BLACK; // Caso 3
            w->color = RED; // Caso 3
            left_rotate(T, w); // Caso 3
            w = x->parent->left; // Caso 3
        }
        w->color = x->parent->color; // Caso 4
        x->parent->color = BLACK; // Caso 4
        w->left->color = BLACK; // Caso 4
        right_rotate(T, x->parent); // Caso 4
    }
}

node_t *delete(RB_tree *T, node_t *z) {
    node_t *y;
    if (z->left == NULL || z->right == NULL)
        y = z;
    else
        y = tree_successor(z);
    node_t *x;
    if (y->left != NULL)
        x = y->left;
    else x = y->right;
    x->parent = y->parent; // x potrebbe essere T.nil;
    if (y->parent == NULL)
        *T = x;
    else if (y == y->parent->left)
        y->parent->left = x;
    else y->parent->right = x;
    if (y != z)
        z->word = y->word;
    if (y->color == BLACK)
        delete_fixup(T, x);
    return y;
}

node_t *copy(const node_t *x, node_t *p, int k) {
    if (x != NULL) {
        node_t *node = malloc(sizeof(node_t));
        node->color = x->color;
        node->parent = p;
        node->word = malloc(sizeof(char[k]));
        strcpy(node->word, x->word);
        node->left = copy(x->left, node, k);
        node->right = copy(x->right, node, k);
        return node;
    }
    return NULL;
}

void nuova_partita(const RB_tree *tree, int k) {
    char ref_word[k];
    if (!scanf("%s", ref_word)) return;
    int n;
    if (!scanf("%d", &n)) return;
    RB_tree filtered_tree = copy(*tree, NULL, k);
    printTree(filtered_tree);
    while (n > 0) {
        char word[k];
        if (scanf("%s", word) < 0)break;
        if (search(*tree, word)) {
            char res[k];
            char used[k];
            for (int j = 0; j < k; j++) {
                if (ref_word[j] == word[j]) {
                    res[j] = '+';
                    used[j] = 1;
                }
            }

            for (int j = 0; j < k; j++) {
                if (res[j] != '+') {
                    int found = 0;
                    for (int i = 0; i < k; i++) {
                        if (ref_word[i] == word[j] && used[i] != 1) {
                            res[j] = '|';
                            used[i] = 1;
                            found = 1;
                        }
                    }
                    if (!found)
                        res[j] = '/';
                }
            }
            for (int j = 0; j < k; j++) {
                printf("%c", res[j]);
            }
            printf("\n");
            n--;
        } else {
            printf("not_exist\n");
        }
    }
}

int main() {
    RB_tree t = NULL;
    node_t *x;
    int k;
    if (scanf("%d", &k)) {
        while (1) {
            char temp[256];
            if (scanf("%s", temp) < 0)break;
            if (strcmp(temp, "+nuova_partita") == 0)
                nuova_partita(&t, k);
            if (strlen(temp) == k) {
                char *c = malloc(sizeof(char[k]));
                strcpy(c, temp);
                x = malloc(sizeof(node_t));
                x->word = c;
                insert(&t, x);
            }
        }
    }
    return 0;
}
