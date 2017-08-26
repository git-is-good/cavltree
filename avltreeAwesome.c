#include "avltreeAwesome.h"
#include <stdlib.h>
#include <assert.h>
#include <sys/types.h>
#include <stdio.h>
#include <time.h>
#include <unistd.h>

#define DEBUG_AVLTREE

#define MAXOFTWO(x, y) ((x) > (y) ? (x) : (y))

static void
_destroyTreeNode(TreeNode *nd)
{
    if ( !nd ) return;
    _destroyTreeNode(nd->left);
    _destroyTreeNode(nd->right);
    FREEKEY(nd->key);
    free(nd);
}

void
destroyAVLTree(AVLTree *tr)
{
    _destroyTreeNode(tr->root);
    free(tr);
}

static TreeNode*
_createTreeNode(TreeNode *parent, const KEYTYPE key, VALUETYPE value)
{
    TreeNode *nd = (TreeNode*) malloc(sizeof(TreeNode));
    nd->parent = parent;
    nd->left = NULL;
    nd->right = NULL;
    nd->height = 1;
    DEEPCOPYKEY(nd->key, key);
    nd->value = value;
    return nd;
}

// precondition: nd != NULL
// return whether a insert really happened
static int
_putKeyValueInNode(TreeNode *nd, const KEYTYPE key, VALUETYPE value, TreeNode **insertedNode)
{
    assert(nd);
    COMPARESIGNTYPE cpr = COMPAREKEYS(key, nd->key);
    if ( cpr < 0 ){
        if ( nd->left ) return _putKeyValueInNode(nd->left, key, value, insertedNode);
        *insertedNode = _createTreeNode(nd, key, value);
        nd->left = *insertedNode;
        return 1;
    }else if ( cpr == 0 ){
        nd->value = value;
        *insertedNode = NULL; // defensive
        return 0;
    }else{
        if ( nd->right ) return _putKeyValueInNode(nd->right, key, value, insertedNode);
        *insertedNode = _createTreeNode(nd, key, value);
        nd->right = *insertedNode;
        return 1;
    }
}

/***** A and C must exsit *****/
//     A                  C
//  B    C      --->   A    E
//     D   E         B   D
static TreeNode*
_leftRotate(TreeNode *nd)
{
    assert(nd && nd->right);
    TreeNode *a = nd, *c = nd->right, *d = c->left;
    a->height = MAXOFTWO(HEIGHTOF(a->left), HEIGHTOF(d)) + 1;
    c->height = MAXOFTWO(HEIGHTOF(c->right), a->height) + 1;
    REPLACECHD(nd, c);
    SETPRTLEFTCHD(c, a);
    SETPRTRIGHTCHD(a, d);
    return c;
}

/***** A and B must exsit *****/
//     A                    B
//   B   C      ---->     D   A
//  D E                      E C
static TreeNode*
_rightRotate(TreeNode *nd)
{
    assert(nd && nd->left);
    TreeNode *a = nd, *b = a->left, *e = b->right;
    a->height = MAXOFTWO(HEIGHTOF(a->right), HEIGHTOF(e)) + 1;
    b->height = MAXOFTWO(HEIGHTOF(b->left), a->height) + 1;
    REPLACECHD(nd, b);
    SETPRTRIGHTCHD(b, a);
    SETPRTLEFTCHD(a, e);
    return b;
}

void
putKeyValue(AVLTree *tr, const KEYTYPE key, VALUETYPE value)
{
    if ( !tr->root ){
        tr->root = _createTreeNode(NULL, key, value);
        tr->curSize = 1;
        return;
    }
    TreeNode *insertedNode;
    
    // only an update
    if ( !_putKeyValueInNode(tr->root, key, value, &insertedNode) ) return;
    
    tr->curSize++;
    
    // recover from put
    TreeNode *cur = insertedNode->parent;
    while ( cur ){
        cur->height = MAXOFTWO(HEIGHTOF(cur->left), HEIGHTOF(cur->right)) + 1;
        if ( NEEDLEFTROTA(cur) ){
            if ( HEIGHTOF(cur->right->left) > HEIGHTOF(cur->right->right) ){
                _rightRotate(cur->right);
            }
            if ( cur == tr->root ){
                tr->root = _leftRotate(cur);
            }else{
                _leftRotate(cur);
            }
            return;
        }else if ( NEEDRIGHTROTA(cur) ){
            if ( HEIGHTOF(cur->left->right) > HEIGHTOF(cur->left->left) ){
                _leftRotate(cur->left);
            }
            if ( cur == tr->root ){
                tr->root = _rightRotate(cur);
                return;
            }else{
                _rightRotate(cur);
            }
            return;
        }
        cur = cur->parent;
    }
}

// return the new node when key is removed.
// invariant: every field except the parent is handled
//            do NOTHING for parent
static TreeNode*
_removeKeyInNode(TreeNode *nd, const KEYTYPE key, int *isrmd)
{
    if ( !nd ){
        isrmd = 0;
        return NULL;
    }
    
    COMPARESIGNTYPE cpr = COMPAREKEYS(key, nd->key);
    if ( cpr < 0 ){
        nd->left = _removeKeyInNode(nd->left, key, isrmd);
        if ( nd->left ) nd->left->parent = nd;
    }else if ( cpr > 0 ){
        nd->right = _removeKeyInNode(nd->right, key, isrmd);
        if ( nd->right ) nd->right->parent = nd;
    }else{
        if ( !nd->right ){
            // nearly empty
            FREEKEY(nd->key);
            *isrmd = 1;
            TreeNode *res = nd->left;
            free(nd);
            return res;
        }
        if ( !nd->right->left ){
            TreeNode *cur = nd->right, *ndl = nd->left;
            REPLACECHD(nd, cur);
            SETPRTLEFTCHD(cur, ndl);
            FREEKEY(nd->key);
            free(nd);
            *isrmd = 1;
            nd = cur;
        }else{
            // find the leftmost of the right child subtree
            // exchange this leftmost with nd
            // note: after change, nd->right is still a BST,
            //      but temporarily, nd is not.
            TreeNode *cur = nd->right->left;
            while ( cur->left ){
                cur = cur->left;
            }
            TreeNode *curp = cur->parent, *curr = cur->right,
            *ndl = nd->left, *ndr = nd->right;
            REPLACECHD(nd, cur);
            SETPRTLEFTCHD(curp, nd);
            
            SETPRTLEFTCHD(cur, ndl);
            SETPRTRIGHTCHD(cur, ndr);
            cur->height = nd->height;
            
            nd->left = NULL;
            SETPRTRIGHTCHD(nd, curr);
            nd->height = 1;
            
            nd = cur;
            nd->right = _removeKeyInNode(nd->right, key, isrmd);
            if ( nd->right ) nd->right->parent = nd;
        }
    }
    
    if ( NEEDLEFTROTA(nd) ){
        if ( HEIGHTOF(nd->right->left) > HEIGHTOF(nd->right->right) ){
            nd->right = _rightRotate(nd->right);
        }
        return _leftRotate(nd);
    }else if ( NEEDRIGHTROTA(nd) ){
        if ( HEIGHTOF(nd->left->right) > HEIGHTOF(nd->left->left) ){
            nd->left = _leftRotate(nd->left);
        }
        return _rightRotate(nd);
    }else{
        nd->height = MAXOFTWO(HEIGHTOF(nd->right), HEIGHTOF(nd->left)) + 1;
        return nd;
    }
}


void
removeKey(AVLTree *tr, const KEYTYPE key)
{
    int isrmd = 0;
    tr->root = _removeKeyInNode(tr->root, key, &isrmd);
    if ( isrmd ) tr->curSize--;
}


static inline TreeNode*
_findKeyInNode(TreeNode *nd, const KEYTYPE key)
{
    while ( nd ){
        COMPARESIGNTYPE cpr = COMPAREKEYS(key, nd->key);
        if ( cpr < 0 ){
            nd = nd->left;
        }else if ( cpr == 0 ){
            return nd;
        }else{
            nd = nd->right;
        }
    }
    return NULL;
}

TreeNode*
findKey(AVLTree *tr, const KEYTYPE key)
{
    return _findKeyInNode(tr->root, key);
}

TreeNode*
firstTreeNode(AVLTree *tr){
    TreeNode *cur = tr->root;
    if ( !cur ) return NULL;
    while ( cur->left ){
        cur = cur->left;
    }
    return cur;
}

TreeNode*
nextTreeNode(TreeNode *nd)
{
    if ( !nd ) return NULL;
    if ( nd->right ){
        TreeNode *cur = nd->right;
        while ( cur->left ){
            cur = cur->left;
        }
        return cur;
    }else{
        while ( nd->parent && ISRIGHTCHILD(nd) ){
            nd = nd->parent;
        }
        return nd->parent;
    }
}


#ifdef DEBUG_AVLTREE

static void
checkAVLProperty(TreeNode *nd)
{
    if ( !nd ) return;
    if ( nd->left ) {
        assert( COMPAREKEYS(nd->left->key, nd->key) < 0 );
        assert(nd->left->parent == nd);
    }
    if ( nd->right ){
        assert( COMPAREKEYS(nd->key, nd->right->key) < 0 );
        assert(nd->right->parent == nd);
    }
    assert(HEIGHTOF(nd) == MAXOFTWO(HEIGHTOF(nd->right), HEIGHTOF(nd->left)) + 1);
    assert(!NEEDRIGHTROTA(nd) && !NEEDLEFTROTA(nd));
    checkAVLProperty(nd->left);
    checkAVLProperty(nd->right);
}

static void
printAVLTree(TreeNode *nd){
    if ( !nd ) return;
    printAVLTree(nd->left);
    printf("key = %lu, value = %p\n", nd->key, nd->value);
    printAVLTree(nd->right);
}

static void
test1()
{
    AVLTree *tr = createAVLTree();
    const size_t testSetSize = 10000000;
    
    TreeNode *tnd = firstTreeNode(tr);
    assert(tnd == NULL);
    for ( size_t i = 0; i < 10; i++ ){
        removeKey(tr, i);
    }
    assert(getAVLTreeSize(tr) == 0);
    for ( size_t i = 0; i < testSetSize; i++ ){
        putKeyValue(tr, i, (VALUETYPE)i);
    }
    for ( size_t i = 0; i < testSetSize - 1; i++ ){
        removeKey(tr, i);
    }
    assert(tr->root != NULL);
    removeKey(tr, testSetSize-1);
    assert(tr->root == NULL);
    assert(getAVLTreeSize(tr) == 0);
    tnd = firstTreeNode(tr);
    assert(tnd == NULL);
    
    
    clock_t clc = clock();
    for ( size_t i = 0; i < testSetSize; i++ ){
        putKeyValue(tr, i, (VALUETYPE)i);
    }
    printf("awe putting done...%ld\n", clock() - clc);
    assert(getAVLTreeSize(tr) == testSetSize);
    
    clc = clock();
    for ( size_t i = 0; i < testSetSize; i++ ){
        assert(findValueByKey(tr, i) == (VALUETYPE)i);
    }
    printf("awe finding done...%ld\n", clock() - clc);
    //printAVLTree(tr->root);

    clc = clock();
    tnd = firstTreeNode(tr);
    size_t cnt = 0;
    while ( tnd ){
        assert(tnd->value == (VALUETYPE)cnt);
        cnt++;
        tnd = nextTreeNode(tnd);
    }
    printf("awe traversing done...%ld\n", clock() - clc);
    assert(cnt == testSetSize);
    checkAVLProperty(tr->root);
    
    clc = clock();
    for ( size_t i = testSetSize/3; i < testSetSize/2; i++ ){
        removeKey(tr, i);
    }
    printf("awe removing done...%ld\n", clock() - clc);
    
    for ( size_t i = testSetSize; i < 2 * testSetSize; i++ ){
        removeKey(tr, i);
    }
    checkAVLProperty(tr->root);
    
    clc = clock();
    destroyAVLTree(tr);
    printf("awe destroying done...%ld\n", clock() - clc);

    printf("done...\n");
}

static void
test2()
{
    AVLTree *tr = createAVLTree();
    const size_t testSetSize = 10000000;
    
    for ( size_t i = 0; i < testSetSize; i++ ){
        putKeyValue(tr, i, (VALUETYPE)i);
    }
    
    clock_t clc = clock();
    for ( size_t i = testSetSize/3; i < testSetSize/2; i++ ){
        removeKey(tr, i);
    }
    printf("awe removing done...%ld\n", clock() - clc);
    checkAVLProperty(tr->root);
    
    for ( size_t i = testSetSize/3; i < testSetSize/2; i++ ){
        putKeyValue(tr, i, (VALUETYPE)i);
    }
    
    clc = clock();
    TreeNode *tnd = firstTreeNode(tr);
    size_t cnt = 0;
    while ( tnd ){
        assert(tnd->value == (VALUETYPE)cnt);
        cnt++;
        tnd = nextTreeNode(tnd);
    }
    printf("awe traversing done...%ld\n", clock() - clc);
    assert(cnt == testSetSize);
    checkAVLProperty(tr->root);
    //printAVLTree(tr->root);
    
    destroyAVLTree(tr);
    printf("done...\n");
}

static void
testRemoveLeak()
{
    AVLTree *tr = createAVLTree();
    const size_t testSetSize = 10000000;
    
    for ( size_t i = 0; i < testSetSize; i++ ){
        putKeyValue(tr, i, (VALUETYPE)i);
    }
    
    for ( int i = 0; i < 1000; i++ ){
        for ( size_t i = testSetSize/3; i < testSetSize/2; i++ ){
            removeKey(tr, i);
        }
        for ( size_t i = testSetSize/3; i < testSetSize/2; i++ ){
            removeKey(tr, i);
        }
    }
    destroyAVLTree(tr);
}

int
main()
{
    test1();
    //testRemoveLeak();
    pause();
}

#endif
