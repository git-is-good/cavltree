#ifndef avltreeAwesome_h
#define avltreeAwesome_h

#include <stdlib.h>
#include <sys/types.h>

#define VALUETYPE                    void*
#define NONEXISTVALUE                NULL

#define KEYTYPE                      unsigned long
#define DEEPCOPYKEY(dstKey, srcKey)  (dstKey) = (KEYTYPE)(srcKey);
#define FREEKEY(key)
#define COMPARESIGNTYPE              long
#define COMPAREKEYS(key1, key2)      ((COMPARESIGNTYPE)((key1) - (key2)))

// here can be further optimized for space complexity
// but note that putKeyValue SHOULD NEVER be implemented by recursion.
// A recursive version slows down about 2 times comparing to this non-recursive version ... (this version is 1 time faster than C++ std, with almost the same memory cost)
// for such optimization, a dynamic Vector is needed ...
typedef struct TreeNode {
    struct TreeNode *parent;
    struct TreeNode *left;
    struct TreeNode *right;
    size_t          height;
    KEYTYPE         key;
    VALUETYPE       value;
}TreeNode;

#define HEIGHTOF(nd) ((nd) ? (nd)->height : 0)
#define NEEDRIGHTROTA(nd)                                \
     (HEIGHTOF((nd)->left) > HEIGHTOF((nd)->right) + 1)
#define NEEDLEFTROTA(nd)                                 \
     (HEIGHTOF((nd)->right) > HEIGHTOF((nd)->left) + 1)

#define ISLEFTCHILD(nd)  ((nd)->parent->left == (nd))
#define ISRIGHTCHILD(nd) ((nd)->parent->right == (nd))

#define SETPRTLEFTCHD(prt, lchd)     \
    (prt)->left = (lchd);            \
    if (lchd) (lchd)->parent = (prt);

#define SETPRTRIGHTCHD(prt, rchd)    \
    (prt)->right = (rchd);           \
    if (rchd) (rchd)->parent = (prt);

#define REPLACECHD(oldChd, newChd)                      \
    if ( oldChd->parent ){                              \
        if ( ISLEFTCHILD( oldChd ) ){                   \
            SETPRTLEFTCHD( oldChd->parent, newChd );    \
        }else{                                          \
            SETPRTRIGHTCHD( oldChd->parent, newChd) ;   \
        }                                               \
    }else{                                              \
        newChd->parent = NULL;                          \
    }

typedef struct {
    TreeNode *root;
    size_t   curSize;
}AVLTree;

static inline AVLTree *createAVLTree(){
    return (AVLTree*) calloc(1, sizeof(AVLTree));
}
void destroyAVLTree(AVLTree *tr);

void removeKey(AVLTree *tr, const KEYTYPE key);
void putKeyValue(AVLTree *tr, const KEYTYPE key, VALUETYPE value);

TreeNode *findKey(AVLTree *tr, const KEYTYPE key);
static inline int hasKey(AVLTree *tr, const KEYTYPE key){
    return findKey(tr, key) != NULL;
}
static inline VALUETYPE findValueByKey(AVLTree *tr, const KEYTYPE key){
    TreeNode *nd = findKey(tr, key);
    if ( nd ) return nd->value;
    return (VALUETYPE)NONEXISTVALUE;
}

static inline size_t getAVLTreeSize(AVLTree *tr){
    return tr->curSize;
}

// for a tree, maybe this naive concept is better than an iterator...
TreeNode *firstTreeNode(AVLTree *tr);
TreeNode *nextTreeNode(TreeNode *nd);



#endif /* avltreeAwesome_h */
