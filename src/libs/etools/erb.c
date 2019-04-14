/// =====================================================================================
///
///       Filename:  erb.c
///
///    Description:  easy rb_tree, rebuild from linux-4.6.3
///
///        Version:  1.0
///        Created:  03/09/2017 08:51:34 PM
///       Revision:  none
///       Compiler:  gcc
///            SRC:  linux-4.6.3/include/linux/rbtree.h
///                  linux-4.6.3/include/linux/rbtree_augmented.h
///                  linux-4.6.3/lib/rbtree.c
///
///         Author:  Haitao Yang, joyhaitao@foxmail.com
///        Company:
///
/// =====================================================================================

#include "erb.h"

#define __KERNEL__
#include "compiler.h"

#undef  WRITE_ONCE
#define WRITE_ONCE(x, val) x = val

/// =====================================================================================
/// linux-4.6.3/include/linux/rbtree.h
/// =====================================================================================

#undef  NULL
#define NULL ((void *)0)

#undef  container_of
#undef  offsetof

#ifdef _CHECK_TYPE
#define container_of(ptr, type, member) ({const typeof( ((type *)0)->member ) *__mptr = (ptr);(type *)( (char *)__mptr - offsetof(type,member) );})
#define offsetof(TYPE, MEMBER)          ((size_t) &((TYPE *)0)->MEMBER)
#else
#define container_of(ptr, type, member) ((type *)( (char *)ptr - offsetof(type,member) ))
#define offsetof(TYPE, MEMBER)          ((size_t) &((TYPE *)0)->MEMBER)
#endif

typedef struct rb_node {
    unsigned long  __rb_parent_color;
    struct rb_node *rb_right;
    struct rb_node *rb_left;
}rb_node;

typedef struct rb_root {
    struct rb_node *rb_node;
}rb_root;

#define rb_parent(r)   ((struct rb_node *)((r)->__rb_parent_color & ~3))
#define RB_ROOT	{ NULL, }
#define	rb_entry(ptr, type, member) container_of(ptr, type, member)

#define RB_EMPTY_ROOT(root)  (READ_ONCE((root)->rb_node) == NULL)

/* 'empty' nodes are nodes that are known not to be inserted in an rbtree */
#define RB_EMPTY_NODE(node)  ((node)->__rb_parent_color == (unsigned long)(node))
#define RB_CLEAR_NODE(node)  ((node)->__rb_parent_color =  (unsigned long)(node))

static inline void rb_insert_color(struct rb_node *, struct rb_root *);
static inline void rb_erase(struct rb_node *, struct rb_root *);


/* Find logical next and previous nodes in a tree */
static inline rb_node *rb_next(const struct rb_node *);
static inline rb_node *rb_prev(const struct rb_node *);
static inline rb_node *rb_first(const struct rb_root *);
static inline rb_node *rb_last(const struct rb_root *);

/* Postorder iteration - always visit the parent after its children */
static inline struct rb_node *rb_first_postorder(const struct rb_root *);
static inline struct rb_node *rb_next_postorder(const struct rb_node *);

/* Fast replacement of a single node without remove/rebalance/add/rebalance */
static inline void rb_replace_node(struct rb_node *victim, struct rb_node *_new, struct rb_root *root);

static inline void rb_link_node(struct rb_node *node, struct rb_node *parent, struct rb_node **rb_link)
{
    node->__rb_parent_color = (unsigned long)parent;
    node->rb_left = node->rb_right = NULL;

    *rb_link = node;
}

#define rb_entry_safe(ptr, type, member) \
    ({ typeof(ptr) ____ptr = (ptr); \
       ____ptr ? rb_entry(____ptr, type, member) : NULL; \
    })

#define rbtree_postorder_for_each_entry_safe(pos, n, root, field) \
    for (pos = rb_entry_safe(rb_first_postorder(root), typeof(*pos), field); \
         pos && ({ n = rb_entry_safe(rb_next_postorder(&pos->field), \
            typeof(*pos), field); 1; }); \
         pos = n)

/// =====================================================================================
/// linux-4.6.3/include/linux/rbtree_augmented.h
/// =====================================================================================

#ifndef _LINUX_RBTREE_AUGMENTED_H
#define _LINUX_RBTREE_AUGMENTED_H

struct rb_augment_callbacks {
    void (*propagate)(struct rb_node *node, struct rb_node *stop);
    void (*copy)(struct rb_node *old, struct rb_node *_new);
    void (*rotate)(struct rb_node *old, struct rb_node *_new);
};

extern void __rb_insert_augmented(struct rb_node *node, struct rb_root *root,
    void (*augment_rotate)(struct rb_node *old, struct rb_node *_new));

static inline void
rb_insert_augmented(struct rb_node *node, struct rb_root *root,
            const struct rb_augment_callbacks *augment)
{
    __rb_insert_augmented(node, root, augment->rotate);
}

#define RB_DECLARE_CALLBACKS(rbstatic, rbname, rbstruct, rbfield,	\
                 rbtype, rbaugmented, rbcompute)		\
static inline void							\
rbname ## _propagate(struct rb_node *rb, struct rb_node *stop)		\
{									\
    while (rb != stop) {						\
        rbstruct *node = rb_entry(rb, rbstruct, rbfield);	\
        rbtype augmented = rbcompute(node);			\
        if (node->rbaugmented == augmented)			\
            break;						\
        node->rbaugmented = augmented;				\
        rb = rb_parent(&node->rbfield);				\
    }								\
}									\
static inline void							\
rbname ## _copy(struct rb_node *rb_old, struct rb_node *rb_new)		\
{									\
    rbstruct *old  = rb_entry(rb_old, rbstruct, rbfield);		\
    rbstruct *_new = rb_entry(rb_new, rbstruct, rbfield);		\
    _new->rbaugmented = old->rbaugmented;				\
}									\
static void								\
rbname ## _rotate(struct rb_node *rb_old, struct rb_node *rb_new)	\
{									\
    rbstruct *old  = rb_entry(rb_old, rbstruct, rbfield);		\
    rbstruct *_new = rb_entry(rb_new, rbstruct, rbfield);		\
    _new->rbaugmented = old->rbaugmented;				\
    old->rbaugmented  = rbcompute(old);				\
}									\
rbstatic const struct rb_augment_callbacks rbname = {			\
    rbname ## _propagate, rbname ## _copy, rbname ## _rotate	\
};


#define	RB_RED		0
#define	RB_BLACK	1

#define __rb_parent(pc)    ((struct rb_node *)(pc & ~3))

#define __rb_color(pc)     ((pc) & 1)
#define __rb_is_black(pc)  __rb_color(pc)
#define __rb_is_red(pc)    (!__rb_color(pc))
#define rb_color(rb)       __rb_color((rb)->__rb_parent_color)
#define rb_is_red(rb)      __rb_is_red((rb)->__rb_parent_color)
#define rb_is_black(rb)    __rb_is_black((rb)->__rb_parent_color)

static __always_inline void rb_set_parent(struct rb_node *rb, struct rb_node *p)
{
    rb->__rb_parent_color = rb_color(rb) | (unsigned long)p;
}

static __always_inline void rb_set_parent_color(struct rb_node *rb, struct rb_node *p, int color)
{
    rb->__rb_parent_color = (unsigned long)p | color;
}

static inline void
__rb_change_child(struct rb_node *old, struct rb_node *_new,
          struct rb_node *parent, struct rb_root *root)
{
    if (parent) {
        if (parent->rb_left == old)
            WRITE_ONCE(parent->rb_left, _new);
        else
            WRITE_ONCE(parent->rb_right, _new);
    } else
        WRITE_ONCE(root->rb_node, _new);
}

extern void __rb_erase_color(struct rb_node *parent, struct rb_root *root,
    void (*augment_rotate)(struct rb_node *old, struct rb_node *_new));

static __always_inline struct rb_node *
__rb_erase_augmented(struct rb_node *node, struct rb_root *root,
             const struct rb_augment_callbacks *augment)
{
    struct rb_node *child = node->rb_right;
    struct rb_node *tmp = node->rb_left;
    struct rb_node *parent, *rebalance;
    unsigned long pc;

    if (!tmp) {
        /*
         * Case 1: node to erase has no more than 1 child (easy!)
         *
         * Note that if there is one child it must be red due to 5)
         * and node must be black due to 4). We adjust colors locally
         * so as to bypass __rb_erase_color() later on.
         */
        pc = node->__rb_parent_color;
        parent = __rb_parent(pc);
        __rb_change_child(node, child, parent, root);
        if (child) {
            child->__rb_parent_color = pc;
            rebalance = NULL;
        } else
            rebalance = __rb_is_black(pc) ? parent : NULL;
        tmp = parent;
    } else if (!child) {
        /* Still case 1, but this time the child is node->rb_left */
        tmp->__rb_parent_color = pc = node->__rb_parent_color;
        parent = __rb_parent(pc);
        __rb_change_child(node, tmp, parent, root);
        rebalance = NULL;
        tmp = parent;
    } else {
        struct rb_node *successor = child, *child2;

        tmp = child->rb_left;
        if (!tmp) {
            /*
             * Case 2: node's successor is its right child
             *
             *    (n)          (s)
             *    / \          / \
             *  (x) (s)  ->  (x) (c)
             *        \
             *        (c)
             */
            parent = successor;
            child2 = successor->rb_right;

            augment->copy(node, successor);
        } else {
            /*
             * Case 3: node's successor is leftmost under
             * node's right child subtree
             *
             *    (n)          (s)
             *    / \          / \
             *  (x) (y)  ->  (x) (y)
             *      /            /
             *    (p)          (p)
             *    /            /
             *  (s)          (c)
             *    \
             *    (c)
             */
            do {
                parent = successor;
                successor = tmp;
                tmp = tmp->rb_left;
            } while (tmp);
            child2 = successor->rb_right;
            WRITE_ONCE(parent->rb_left, child2);
            WRITE_ONCE(successor->rb_right, child);
            rb_set_parent(child, successor);

            augment->copy(node, successor);
            augment->propagate(parent, successor);
        }

        tmp = node->rb_left;
        WRITE_ONCE(successor->rb_left, tmp);
        rb_set_parent(tmp, successor);

        pc = node->__rb_parent_color;
        tmp = __rb_parent(pc);
        __rb_change_child(node, successor, tmp, root);

        if (child2) {
            successor->__rb_parent_color = pc;
            rb_set_parent_color(child2, parent, RB_BLACK);
            rebalance = NULL;
        } else {
            unsigned long pc2 = successor->__rb_parent_color;
            successor->__rb_parent_color = pc;
            rebalance = __rb_is_black(pc2) ? parent : NULL;
        }
        tmp = successor;
    }

    augment->propagate(tmp, NULL);
    return rebalance;
}

static __always_inline void
rb_erase_augmented(struct rb_node *node, struct rb_root *root,
           const struct rb_augment_callbacks *augment)
{
    struct rb_node *rebalance = __rb_erase_augmented(node, root, augment);
    if (rebalance)
        __rb_erase_color(rebalance, root, augment->rotate);
}

#endif	/* _LINUX_RBTREE_AUGMENTED_H */


/// =====================================================================================
/// linux-4.6.3/lib/rbtree.c
/// =====================================================================================

static __always_inline  void rb_set_black(struct rb_node *rb)
{
    rb->__rb_parent_color |= RB_BLACK;
}

static __always_inline struct rb_node *rb_red_parent(struct rb_node *red)
{
    return (struct rb_node *)red->__rb_parent_color;
}

/*
 * Helper function for rotations:
 * - old's parent and color get assigned to new
 * - old gets assigned new as a parent and 'color' as a color.
 */
static __always_inline void
__rb_rotate_set_parents(struct rb_node *old, struct rb_node *_new,
            struct rb_root *root, int color)
{
    struct rb_node *parent = rb_parent(old);
    _new->__rb_parent_color = old->__rb_parent_color;
    rb_set_parent_color(old, _new, color);
    __rb_change_child(old, _new, parent, root);
}

#define true 1

static __always_inline void
__rb_insert(struct rb_node *node, struct rb_root *root,
        void (*augment_rotate)(struct rb_node *old, struct rb_node *_new))
{
    struct rb_node *parent = rb_red_parent(node), *gparent, *tmp;

    while (true) {
        /*
         * Loop invariant: node is red
         *
         * If there is a black parent, we are done.
         * Otherwise, take some corrective action as we don't
         * want a red root or two consecutive red nodes.
         */
        if (!parent) {
            rb_set_parent_color(node, NULL, RB_BLACK);
            break;
        } else if (rb_is_black(parent))
            break;

        gparent = rb_red_parent(parent);

        tmp = gparent->rb_right;
        if (parent != tmp) {	/* parent == gparent->rb_left */
            if (tmp && rb_is_red(tmp)) {
                /*
                 * Case 1 - color flips
                 *
                 *       G            g
                 *      / \          / \
                 *     p   u  -->   P   U
                 *    /            /
                 *   n            n
                 *
                 * However, since g's parent might be red, and
                 * 4) does not allow this, we need to recurse
                 * at g.
                 */
                rb_set_parent_color(tmp, gparent, RB_BLACK);
                rb_set_parent_color(parent, gparent, RB_BLACK);
                node = gparent;
                parent = rb_parent(node);
                rb_set_parent_color(node, parent, RB_RED);
                continue;
            }

            tmp = parent->rb_right;
            if (node == tmp) {
                /*
                 * Case 2 - left rotate at parent
                 *
                 *      G             G
                 *     / \           / \
                 *    p   U  -->    n   U
                 *     \           /
                 *      n         p
                 *
                 * This still leaves us in violation of 4), the
                 * continuation into Case 3 will fix that.
                 */
                tmp = node->rb_left;
                WRITE_ONCE(parent->rb_right, tmp);
                WRITE_ONCE(node->rb_left, parent);
                if (tmp)
                    rb_set_parent_color(tmp, parent, RB_BLACK);
                rb_set_parent_color(parent, node, RB_RED);
                augment_rotate(parent, node);
                parent = node;
                tmp = node->rb_right;
            }

            /*
             * Case 3 - right rotate at gparent
             *
             *        G           P
             *       / \         / \
             *      p   U  -->  n   g
             *     /                 \
             *    n                   U
             */
            WRITE_ONCE(gparent->rb_left, tmp); /* == parent->rb_right */
            WRITE_ONCE(parent->rb_right, gparent);
            if (tmp)
                rb_set_parent_color(tmp, gparent, RB_BLACK);
            __rb_rotate_set_parents(gparent, parent, root, RB_RED);
            augment_rotate(gparent, parent);
            break;
        } else {
            tmp = gparent->rb_left;
            if (tmp && rb_is_red(tmp)) {
                /* Case 1 - color flips */
                rb_set_parent_color(tmp, gparent, RB_BLACK);
                rb_set_parent_color(parent, gparent, RB_BLACK);
                node = gparent;
                parent = rb_parent(node);
                rb_set_parent_color(node, parent, RB_RED);
                continue;
            }

            tmp = parent->rb_left;
            if (node == tmp) {
                /* Case 2 - right rotate at parent */
                tmp = node->rb_right;
                WRITE_ONCE(parent->rb_left, tmp);
                WRITE_ONCE(node->rb_right, parent);
                if (tmp)
                    rb_set_parent_color(tmp, parent,
                                RB_BLACK);
                rb_set_parent_color(parent, node, RB_RED);
                augment_rotate(parent, node);
                parent = node;
                tmp = node->rb_left;
            }

            /* Case 3 - left rotate at gparent */
            WRITE_ONCE(gparent->rb_right, tmp); /* == parent->rb_left */
            WRITE_ONCE(parent->rb_left, gparent);
            if (tmp)
                rb_set_parent_color(tmp, gparent, RB_BLACK);
            __rb_rotate_set_parents(gparent, parent, root, RB_RED);
            augment_rotate(gparent, parent);
            break;
        }
    }
}

/*
 * Inline version for rb_erase() use - we want to be able to inline
 * and eliminate the dummy_rotate callback there
 */
static __always_inline void
____rb_erase_color(struct rb_node *parent, struct rb_root *root,
    void (*augment_rotate)(struct rb_node *old, struct rb_node *_new))
{
    struct rb_node *node = NULL, *sibling, *tmp1, *tmp2;

    while (true) {
        /*
         * Loop invariants:
         * - node is black (or NULL on first iteration)
         * - node is not the root (parent is not NULL)
         * - All leaf paths going through parent and node have a
         *   black node count that is 1 lower than other leaf paths.
         */
        sibling = parent->rb_right;
        if (node != sibling) {	/* node == parent->rb_left */
            if (rb_is_red(sibling)) {
                /*
                 * Case 1 - left rotate at parent
                 *
                 *     P               S
                 *    / \             / \
                 *   N   s    -->    p   Sr
                 *      / \         / \
                 *     Sl  Sr      N   Sl
                 */
                tmp1 = sibling->rb_left;
                WRITE_ONCE(parent->rb_right, tmp1);
                WRITE_ONCE(sibling->rb_left, parent);
                rb_set_parent_color(tmp1, parent, RB_BLACK);
                __rb_rotate_set_parents(parent, sibling, root,
                            RB_RED);
                augment_rotate(parent, sibling);
                sibling = tmp1;
            }
            tmp1 = sibling->rb_right;
            if (!tmp1 || rb_is_black(tmp1)) {
                tmp2 = sibling->rb_left;
                if (!tmp2 || rb_is_black(tmp2)) {
                    /*
                     * Case 2 - sibling color flip
                     * (p could be either color here)
                     *
                     *    (p)           (p)
                     *    / \           / \
                     *   N   S    -->  N   s
                     *      / \           / \
                     *     Sl  Sr        Sl  Sr
                     *
                     * This leaves us violating 5) which
                     * can be fixed by flipping p to black
                     * if it was red, or by recursing at p.
                     * p is red when coming from Case 1.
                     */
                    rb_set_parent_color(sibling, parent,
                                RB_RED);
                    if (rb_is_red(parent))
                        rb_set_black(parent);
                    else {
                        node = parent;
                        parent = rb_parent(node);
                        if (parent)
                            continue;
                    }
                    break;
                }
                /*
                 * Case 3 - right rotate at sibling
                 * (p could be either color here)
                 *
                 *   (p)           (p)
                 *   / \           / \
                 *  N   S    -->  N   Sl
                 *     / \             \
                 *    sl  Sr            s
                 *                       \
                 *                        Sr
                 */
                tmp1 = tmp2->rb_right;
                WRITE_ONCE(sibling->rb_left, tmp1);
                WRITE_ONCE(tmp2->rb_right, sibling);
                WRITE_ONCE(parent->rb_right, tmp2);
                if (tmp1)
                    rb_set_parent_color(tmp1, sibling,
                                RB_BLACK);
                augment_rotate(sibling, tmp2);
                tmp1 = sibling;
                sibling = tmp2;
            }
            /*
             * Case 4 - left rotate at parent + color flips
             * (p and sl could be either color here.
             *  After rotation, p becomes black, s acquires
             *  p's color, and sl keeps its color)
             *
             *      (p)             (s)
             *      / \             / \
             *     N   S     -->   P   Sr
             *        / \         / \
             *      (sl) sr      N  (sl)
             */
            tmp2 = sibling->rb_left;
            WRITE_ONCE(parent->rb_right, tmp2);
            WRITE_ONCE(sibling->rb_left, parent);
            rb_set_parent_color(tmp1, sibling, RB_BLACK);
            if (tmp2)
                rb_set_parent(tmp2, parent);
            __rb_rotate_set_parents(parent, sibling, root,
                        RB_BLACK);
            augment_rotate(parent, sibling);
            break;
        } else {
            sibling = parent->rb_left;
            if (rb_is_red(sibling)) {
                /* Case 1 - right rotate at parent */
                tmp1 = sibling->rb_right;
                WRITE_ONCE(parent->rb_left, tmp1);
                WRITE_ONCE(sibling->rb_right, parent);
                rb_set_parent_color(tmp1, parent, RB_BLACK);
                __rb_rotate_set_parents(parent, sibling, root,
                            RB_RED);
                augment_rotate(parent, sibling);
                sibling = tmp1;
            }
            tmp1 = sibling->rb_left;
            if (!tmp1 || rb_is_black(tmp1)) {
                tmp2 = sibling->rb_right;
                if (!tmp2 || rb_is_black(tmp2)) {
                    /* Case 2 - sibling color flip */
                    rb_set_parent_color(sibling, parent,
                                RB_RED);
                    if (rb_is_red(parent))
                        rb_set_black(parent);
                    else {
                        node = parent;
                        parent = rb_parent(node);
                        if (parent)
                            continue;
                    }
                    break;
                }
                /* Case 3 - right rotate at sibling */
                tmp1 = tmp2->rb_left;
                WRITE_ONCE(sibling->rb_right, tmp1);
                WRITE_ONCE(tmp2->rb_left, sibling);
                WRITE_ONCE(parent->rb_left, tmp2);
                if (tmp1)
                    rb_set_parent_color(tmp1, sibling,
                                RB_BLACK);
                augment_rotate(sibling, tmp2);
                tmp1 = sibling;
                sibling = tmp2;
            }
            /* Case 4 - left rotate at parent + color flips */
            tmp2 = sibling->rb_right;
            WRITE_ONCE(parent->rb_left, tmp2);
            WRITE_ONCE(sibling->rb_right, parent);
            rb_set_parent_color(tmp1, sibling, RB_BLACK);
            if (tmp2)
                rb_set_parent(tmp2, parent);
            __rb_rotate_set_parents(parent, sibling, root,
                        RB_BLACK);
            augment_rotate(parent, sibling);
            break;
        }
    }
}

/* Non-inline version for rb_erase_augmented() use */
void __rb_erase_color(struct rb_node *parent, struct rb_root *root,
    void (*augment_rotate)(struct rb_node *old, struct rb_node *_new))
{
    ____rb_erase_color(parent, root, augment_rotate);
}

/*
 * Non-augmented rbtree manipulation functions.
 *
 * We use dummy augmented callbacks here, and have the compiler optimize them
 * out of the rb_insert_color() and rb_erase() function definitions.
 */

static inline void dummy_propagate(struct rb_node *node __maybe_unused, struct rb_node *stop __maybe_unused) {}
static inline void dummy_copy(struct rb_node *old __maybe_unused, struct rb_node *_new __maybe_unused) {}
static inline void dummy_rotate(struct rb_node *old __maybe_unused, struct rb_node *_new __maybe_unused) {}

static const struct rb_augment_callbacks dummy_callbacks = {
    dummy_propagate, dummy_copy, dummy_rotate
};

static __always_inline void rb_insert_color(struct rb_node *node, struct rb_root *root)
{
    __rb_insert(node, root, dummy_rotate);
}

static __always_inline void rb_erase(struct rb_node *node, struct rb_root *root)
{
    struct rb_node *rebalance;
    rebalance = __rb_erase_augmented(node, root, &dummy_callbacks);
    if (rebalance)
        ____rb_erase_color(rebalance, root, dummy_rotate);
}

/*
 * Augmented rbtree manipulation functions.
 *
 * This instantiates the same __always_inline functions as in the non-augmented
 * case, but this time with user-defined callbacks.
 */

void __rb_insert_augmented(struct rb_node *node, struct rb_root *root,
    void (*augment_rotate)(struct rb_node *old, struct rb_node *_new))
{
    __rb_insert(node, root, augment_rotate);
}

static __always_inline struct rb_node *rb_first(const struct rb_root *root)
{
    struct rb_node	*n;

    n = root->rb_node;
    if (!n)
        return NULL;
    while (n->rb_left)
        n = n->rb_left;
    return n;
}

static __always_inline struct rb_node *rb_last(const struct rb_root *root)
{
    struct rb_node	*n;

    n = root->rb_node;
    if (!n)
        return NULL;
    while (n->rb_right)
        n = n->rb_right;
    return n;
}

static __always_inline struct rb_node *rb_next(const struct rb_node *node)
{
    struct rb_node *parent;

    if (RB_EMPTY_NODE(node))
        return NULL;

    /*
     * If we have a right-hand child, go down and then left as far
     * as we can.
     */
    if (node->rb_right) {
        node = node->rb_right;
        while (node->rb_left)
            node=node->rb_left;
        return (struct rb_node *)node;
    }

    /*
     * No right-hand children. Everything down and left is smaller than us,
     * so any 'next' node must be in the general direction of our parent.
     * Go up the tree; any time the ancestor is a right-hand child of its
     * parent, keep going up. First time it's a left-hand child of its
     * parent, said parent is our 'next' node.
     */
    while ((parent = rb_parent(node)) && node == parent->rb_right)
        node = parent;

    return parent;
}

static __always_inline struct rb_node *rb_prev(const struct rb_node *node)
{
    struct rb_node *parent;

    if (RB_EMPTY_NODE(node))
        return NULL;

    /*
     * If we have a left-hand child, go down and then right as far
     * as we can.
     */
    if (node->rb_left) {
        node = node->rb_left;
        while (node->rb_right)
            node=node->rb_right;
        return (struct rb_node *)node;
    }

    /*
     * No left-hand children. Go up till we find an ancestor which
     * is a right-hand child of its parent.
     */
    while ((parent = rb_parent(node)) && node == parent->rb_left)
        node = parent;

    return parent;
}

static __always_inline void rb_replace_node(struct rb_node *victim, struct rb_node *_new,
             struct rb_root *root)
{
    struct rb_node *parent = rb_parent(victim);

    /* Set the surrounding nodes to point to the replacement */
    __rb_change_child(victim, _new, parent, root);
    if (victim->rb_left)
        rb_set_parent(victim->rb_left, _new);
    if (victim->rb_right)
        rb_set_parent(victim->rb_right, _new);

    /* Copy the pointers/colour from the victim to the replacement */
    *_new = *victim;
}

static __always_inline struct rb_node *rb_left_deepest_node(const struct rb_node *node)
{
    for (;;) {
        if (node->rb_left)
            node = node->rb_left;
        else if (node->rb_right)
            node = node->rb_right;
        else
            return (struct rb_node *)node;
    }
}

static __always_inline struct rb_node *rb_next_postorder(const struct rb_node *node)
{
    const struct rb_node *parent;
    if (!node)
        return NULL;
    parent = rb_parent(node);

    /* If we're sitting on node, we've already seen our children */
    if (parent && node == parent->rb_left && parent->rb_right) {
        /* If we are the parent's left node, go to the parent's right
         * node then all the way down to the left */
        return rb_left_deepest_node(parent->rb_right);
    } else
        /* Otherwise we are the parent's right node, and the parent
         * should be next */
        return (struct rb_node *)parent;
}

static __always_inline  struct rb_node *rb_first_postorder(const struct rb_root *root)
{
    if (!root->rb_node)
        return NULL;

    return rb_left_deepest_node(root->rb_node);
}



/// =====================================================================================
///                                        erb
/// =====================================================================================

#define ERB_VERSION     "erb 1.2.0"

#ifdef _WIN32
#undef offsetof
#endif // _WIN32

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <inttypes.h>

#include "ecompat.h"
#include "eutils.h"

#include "erb.h"
#include "eobj_p.h"

#pragma pack(push, 1)

typedef struct _erb_node_s{
    rb_node         link;
    ekey            key;
    _ehdr_t         hdr;
    eobj_t          obj;
}_erbn_t, * _erbn;

typedef struct _erb_s{

    rb_root         rb;

    i32             keyt;

    eval            prvt;       // private data
    eobj_cmp_ex_cb  cmp;        // compare func
    eobj_rls_ex_cb  rls;        // release func, only have effect on ERAW and EPTR type of obj

}_erb_t, * _erb;

typedef struct erb_root_s{
    rb_node         link;
    ekey            key;
    _ehdr_t         hdr;
    _erb_t          tree;
}_erbr_t, * _erbr;
#pragma pack(pop)

/// -------------------------- micros helper ---------------------------------

#define _CUR_C_TYPE             ERB

#define _DNODE_TYPE             _erbn_t
#define _DNODE_LNK_FIELD        link
#define _DNODE_KEY_FIELD        key
#define _DNODE_HDR_FIELD        hdr
#define _DNODE_OBJ_FIELD        obj

#define _RNODE_TYPE             _erbr_t
#define _RNODE_TYP_FIELD        type
#define _RNODE_OBJ_FIELD        tree

#define _cur_dupkeyS            strdup
#define _cur_cmpkeyS            strcmp
#define _cur_freekeyS           efree

#define _c_r(t)                 container_of(t, _erbr_t, tree)
#define _c_len(l)               _eo_len(l)
#define _c_keys(l)              _eo_keys(l)
#define _c_typecoe(l)           _eo_typecoe(l)
#define _c_typecoe_set(l)       _c_typecoe(l) = _cur_type(_CUR_C_TYPE, COE_OBJ  );
#define _c_free(t)              _r_free(_c_r(t))

#define _c_rb(t)                t->rb
#define _c_rn(t)                _c_rb(t).rb_node
#define _c_keyt(t)              (t)->keyt
#define _c_prvt(t)              (t)->prvt
#define _c_cmp(t)               (t)->cmp
#define _c_rls(t)               (t)->rls

/// ------------------------ inline compat ------------------------
#if defined(_WIN32) && !defined(__cplusplus)
#define INLINE
#ifndef _MSC_VER
#define container_of(ptr, type, member) ((type *)( (char *)ptr - offsetof(type,member) ))
#define offsetof(TYPE, MEMBER)          ((size_t) &((TYPE *)0)->MEMBER)
#endif
#else
#define GCC_VERSION (__GNUC__ * 10000 + __GNUC_MINOR__ * 100 + __GNUC_PATCHLEVEL__)
#if GCC_VERSION < 40500
#define INLINE
#else
#define INLINE inline
#endif
#endif

/// -------------------------- erb APIs ---------------------------

erb erb_new (etypek type)
{
    _erbr r = _r_new();

    _r_typeco_set(r);
    _r_keys(r) = (type & EKEY_S) > 0;

    _c_keyt((erb)_r_o(r)) = type;

    return _r_o(r);
}

bool erb_setType(erb t, etypek type)
{
    is1_ret(!t || 0 == _c_len(t), false);

    _c_keys(t)      = (type & EKEY_S) > 0;
    _c_keyt(t)      = type;

    return true;
}

bool erb_setPrvt(erb t, eval          prvt) { is0_ret( t                  , false); _c_prvt(t) = prvt; return true; }
bool erb_setCmp (erb t, eobj_cmp_ex_cb cmp) { is1_ret(!t || 0 == _c_len(t), false); _c_cmp(t)  = cmp ; return true; }
bool erb_setRls (erb t, eobj_rls_ex_cb rls) { is0_ret( t                  , false); _c_rls(t)  = rls ; return true; }

static void __erb_free_root_nokey(erb t, rb_node* node)
{
    if(node->rb_left ) __erb_free_root_nokey(t, node->rb_left);
    if(node->rb_right) __erb_free_root_nokey(t, node->rb_right);

    efree(_l_n(node));
}

static void __erb_free_root_key(erb t, rb_node* node)
{
    if(node->rb_left ) __erb_free_root_key(t, node->rb_left);
    if(node->rb_right) __erb_free_root_key(t, node->rb_right);

    efree(_l_keyS(node));

    efree(_l_n(node));
}

static void __erb_free_root_nokey_rls(erb t, rb_node* node, eobj_rls_ex_cb rls)
{
    if(node->rb_left ) __erb_free_root_nokey_rls(t, node->rb_left, rls);
    if(node->rb_right) __erb_free_root_nokey_rls(t, node->rb_right, rls);

    rls(_l_o(node), _c_prvt(t));

    efree(_l_n(node));
}

static void __erb_free_root_key_rls(erb t, rb_node* node, eobj_rls_ex_cb rls)
{
    if(node->rb_left ) __erb_free_root_key_rls(t, node->rb_left , rls);
    if(node->rb_right) __erb_free_root_key_rls(t, node->rb_right, rls);

    efree(_l_keyS(node)); rls(_l_o(node), _c_prvt(t));

    efree(_l_n(node));
}

int erb_clear(erb t)
{
    return erb_clearEx(t, 0);
}

int  erb_clearEx(erb t, eobj_rls_ex_cb rls)
{
    int len;

    is0_ret(t, 0);

    len = _c_len(t);

    is0_ret(len, 0);

    if(_c_rn(t))
    {
        if(!rls)
            rls = _c_rls(t);

        if(rls)
        {
            _eo_keys(t) ? __erb_free_root_key_rls  (t, _c_rn(t), rls)
                        : __erb_free_root_nokey_rls(t, _c_rn(t), rls) ;
        }
        else
        {
            _eo_keys(t) ? __erb_free_root_key  (t, _c_rn(t))
                        : __erb_free_root_nokey(t, _c_rn(t)) ;
        }
    }

    _c_len(t) = 0;
    _c_rn(t)  = 0;

    return len;
}

inline
uint  erb_len (erb  t  ) { return t   ? _c_len(t)    : 0;}

int  erb_free(erb t)
{
    return erb_freeEx(t, 0);
}

int erb_freeEx(erb t, eobj_rls_ex_cb rls)
{
    is0_ret(t, 0);

    if(_c_rn(t))
    {
        if(!rls)
            rls = _c_rls(t);

        if(rls)
        {
            _eo_keys(t) ? __erb_free_root_key_rls  (t, _c_rn(t), rls)
                        : __erb_free_root_nokey_rls(t, _c_rn(t), rls) ;
        }
        else
        {
            _eo_keys(t) ? __erb_free_root_key  (t, _c_rn(t))
                        : __erb_free_root_nokey(t, _c_rn(t)) ;
        }
    }

    _c_free(t);

    return 1;
}

eobj erb_newO(etypeo type, uint len)
{
    _erbn n;

    switch (type) {
        case EFALSE : _n_newTF(n); break;
        case ETRUE  : _n_newTT(n); break;
        case ENULL  : _n_newTN(n); break;
        case ENUM   : _n_newIc(n); break;
        case EPTR   : _n_newPc(n); break;
        case ESTR   : _n_newSc(n, len); break;
        case ERAW   : _n_newRc(n, len); break;
        case EOBJ   : _n_newOc(n); break;

        default     : return 0;
    }

    return _n_o(n);
}

typedef struct __pos_s
{
    u8        canadd;
    u8        find;
    rb_node  *parent;
    rb_node **pos;

}__pos_t, *__pos;

static __always_inline void __erb_get_key_pos(erb t, eobj obj, __pos pos, int multi)
{
    eval ret;

    pos->canadd = 0;
    pos->find   = 0;
    pos->parent = NULL;
    pos->pos    = &_c_rn(t);

    if(_c_cmp(t))
    {
        // todo
    }
    else
    {
        switch (_c_keyt(t))
        {
            case EKEY_I :   while(*pos->pos)
                            {
                                pos->parent = *pos->pos;
                                ret.i = _eo_keyI(obj) - _l_keyI(pos->parent);

                                if     (ret.i < 0)          pos->pos = &pos->parent->rb_left;
                                else if(ret.i > 0)          pos->pos = &pos->parent->rb_right;
                                else if(!multi)           { pos->find = 1; return;}
                                else                      { pos->find = 1; pos->pos = &pos->parent->rb_right;  }
                            }
                            break;

            case EKEY_U :   while(*pos->pos)
                            {
                                pos->parent = *pos->pos;
                                ret.i = _eo_keyU(obj) - _l_keyU(pos->parent);

                                if     (ret.i < 0)          pos->pos = &pos->parent->rb_left;
                                else if(ret.i > 0)          pos->pos = &pos->parent->rb_right;
                                else if(!multi)           { pos->find = 1; return;}
                                else                      { pos->find = 1; pos->pos = &pos->parent->rb_right;  }
                            }
                            break;

            case EKEY_F :   while(*pos->pos)
                            {
                                pos->parent = *pos->pos;
                                ret.f = _eo_keyF(obj) - _l_keyF(pos->parent);

                                if     (ret.f < 0)          pos->pos = &pos->parent->rb_left;
                                else if(ret.f > 0)          pos->pos = &pos->parent->rb_right;
                                else if(!multi)           { pos->find = 1; return;}
                                else                      { pos->find = 1; pos->pos = &pos->parent->rb_right;  }
                            }
                            break;

            case EKEY_I | EKEY_DES :
                            while(*pos->pos)
                            {
                                pos->parent = *pos->pos;
                                ret.i = _l_keyI(pos->parent) - _eo_keyI(obj);

                                if     (ret.i < 0)          pos->pos = &pos->parent->rb_left;
                                else if(ret.i > 0)          pos->pos = &pos->parent->rb_right;
                                else if(!multi)           { pos->find = 1; return;}
                                else                      { pos->find = 1; pos->pos = &pos->parent->rb_right;  }
                            }
                            break;

            case EKEY_U | EKEY_DES :
                            while(*pos->pos)
                            {
                                pos->parent = *pos->pos;
                                ret.i = _l_keyU(pos->parent) - _eo_keyU(obj);

                                if     (ret.i < 0)          pos->pos = &pos->parent->rb_left;
                                else if(ret.i > 0)          pos->pos = &pos->parent->rb_right;
                                else if(!multi)           { pos->find = 1; return;}
                                else                      { pos->find = 1; pos->pos = &pos->parent->rb_right;  }
                            }
                            break;

            case EKEY_F | EKEY_DES :
                            while(*pos->pos)
                            {
                                pos->parent = *pos->pos;
                                ret.f = _l_keyF(pos->parent) - _eo_keyF(obj);

                                if     (ret.f < 0)          pos->pos = &pos->parent->rb_left;
                                else if(ret.f > 0)          pos->pos = &pos->parent->rb_right;
                                else if(!multi)           { pos->find = 1; return;}
                                else                      { pos->find = 1; pos->pos = &pos->parent->rb_right;  }
                            }
                            break;

            case EKEY_S :   is0_ret(_eo_keyS(obj), );

                            while(*pos->pos)
                            {
                                pos->parent = *pos->pos;
                                ret.i = strcmp(_eo_keyS(obj), _l_keyS(pos->parent));

                                if     (ret.i < 0)          pos->pos = &pos->parent->rb_left;
                                else if(ret.i > 0)          pos->pos = &pos->parent->rb_right;
                                else if(!multi)           { pos->find = 1; return;}
                                else                      { pos->find = 1; pos->pos = &pos->parent->rb_right;  }
                            }

                            break;

            case EKEY_S | EKEY_DES:
                            is0_ret(_eo_keyS(obj), );

                            while(*pos->pos)
                            {
                                pos->parent = *pos->pos;
                                ret.i = strcmp(_l_keyS(pos->parent), _eo_keyS(obj));

                                if     (ret.i < 0)          pos->pos = &pos->parent->rb_left;
                                else if(ret.i > 0)          pos->pos = &pos->parent->rb_right;
                                else if(!multi)           { pos->find = 1; return;}
                                else                      { pos->find = 1; pos->pos = &pos->parent->rb_right;  }
                            }

                            break;
        }
    }

    pos->canadd = 1;
}

static eobj __erb_makeRoom(erb t, eobj obj, int multi, int overwrite)
{
    _erbn n; __pos_t pos;

    is0_ret(t, 0);

    // -- get position
    __erb_get_key_pos(t, obj, &pos, multi);

    if(overwrite && pos.find)
    {
        uint cur_len;

        n = _l_n(pos.parent);

        switch(_n_typeo(n))
        {
            case EFALSE:
            case ETRUE :
            case ENULL : cur_len = _n_len(n); break;
            case ENUM  : cur_len = 8; break;
            case EPTR  : cur_len = 8; break;
            case ESTR  : cur_len = _n_len(n) + 1; break;
            case ERAW  : cur_len = _n_len(n) + 1; break;
            case EOBJ  : cur_len = 8;
                         erb_free(_n_valP(n)); break;
            default:
                return 0;
        }

        if(cur_len < _eo_len(obj))
        {
            _erbn newn = _n_newr(n, _eo_len(obj));

            if(newn != n)
            {
                *pos.pos = &_n_l(newn);

                n = newn;
            }
        }
    }
    else
    {
        if(!pos.canadd)
            return 0;

        // -- make node
        is0_ret(n = _n_newm(_eo_len(obj)), 0);
        _n_init(n);

        if(_eo_keys(t)) _eo_keyS(obj) = _cur_dupkeyS(_eo_keyS(obj));

        _n_key(n) = _eo_key(obj);

        // -- link to rbtree
        rb_link_node(&_n_l(n), pos.parent, pos.pos);
        rb_insert_color(&_n_l(n), &_c_rb(t));

        _c_len(t)++;
    }

    return _n_o(n);
}

static __always_inline eobj __erb_addO(erb r, ekey key, eobj obj, int multi)
{
    rb_node **pos, * parent; i64 ret;

    pos      = &_c_rn(r);
    parent   = NULL;

    if(_eo_keys(r))
    {
        while(*pos)
        {
            parent = *pos;
            ret = strcmp(_k_keyS(key), _l_keyS(parent));

            if     (ret < 0)            pos = &parent->rb_left;
            else if(ret > 0)            pos = &parent->rb_right;
            else if(_c_cmp(r))
            {
                do{
                    parent = *pos;
                    ret = _c_cmp(r)(obj, _l_o(parent), _c_prvt(r));

                    if     (ret < 0)            pos = &parent->rb_left;
                    else if(ret > 0)            pos = &parent->rb_right;
                    else if(!multi)             return NULL;
                    else                        pos = &parent->rb_right;
                }while(*pos);
                break;
            }
            else if(!multi)             return NULL;
            else                        pos = &parent->rb_right;
        }

        _k_keyS(key) = strdup(_k_keyS(key));
    }
    else
    {
        while(*pos)
        {
            parent = *pos;
            ret = _k_keyI(key) -  _l_keyI(parent);

            if     (ret < 0)            pos = &parent->rb_left;
            else if(ret > 0)            pos = &parent->rb_right;
            else if(_c_cmp(r))
            {
                do{
                    parent = *pos;
                    ret = _c_cmp(r)(obj, _l_o(parent), _c_prvt(r));

                    if     (ret < 0)            pos = &parent->rb_left;
                    else if(ret > 0)            pos = &parent->rb_right;
                    else if(!multi)             return NULL;
                    else                        pos = &parent->rb_right;
                }while(*pos);
                break;
            }
            else if(!multi)             return NULL;
            else                        pos = &parent->rb_right;
        }
    }

    _eo_keyS(obj) = (cstr)_k_keyS(key);

    // -- link to rbtree
    rb_link_node(&_eo_l(obj), parent, pos);
    rb_insert_color(&_eo_l(obj), &_c_rb(r));

    _c_len(r)++;

    return obj;
}

eobj erb_addI( erb t, ekey key, i64    val) { _erbn_t b = {{0}, key, {._len =                         8, ._typ = {.t_coe = _ERB_COE_NUM_I}}, {.i =       val} }; eobj o = __erb_makeRoom(t, _n_o(&b), 0, 0); if(o) {              _eo_setI (o, val            ); _eo_typecoe(o) = _ERB_COE_NUM_I; } return o; }
eobj erb_addF( erb t, ekey key, f64    val) { _erbn_t b = {{0}, key, {._len =                         8, ._typ = {.t_coe = _ERB_COE_NUM_F}}, {.f =       val} }; eobj o = __erb_makeRoom(t, _n_o(&b), 0, 0); if(o) {              _eo_setF (o, val            ); _eo_typecoe(o) = _ERB_COE_NUM_F; } return o; }
eobj erb_addP( erb t, ekey key, conptr ptr) { _erbn_t b = {{0}, key, {._len =               sizeof(ptr), ._typ = {.t_coe = _ERB_COE_PTR  }}, {.p = (cptr)ptr} }; eobj o = __erb_makeRoom(t, _n_o(&b), 0, 0); if(o) {              _eo_setP (o, ptr            ); _eo_typeco (o) = _ERB_CO_PTR   ; } return o; }
eobj erb_addR( erb t, ekey key, size   len) { _erbn_t b = {{0}, key, {._len =                   len + 1, ._typ = {.t_coe = _ERB_COE_RAW  }}, {             0} }; eobj o = __erb_makeRoom(t, _n_o(&b), 0, 0); if(o) {              _eo_wipeR(o, len            ); _eo_typeco (o) = _ERB_CO_RAW   ; } return o; }
eobj erb_addS( erb t, ekey key, constr str) { _erbn_t b = {{0}, key, {._len = str ? strlen(str) + 1 : 1, ._typ = {.t_coe = _ERB_COE_STR  }}, {.s = (cstr)str} }; eobj o = __erb_makeRoom(t, _n_o(&b), 0, 0); if(o) {_n_len(&b)--; _eo_setS (o, str, _n_len(&b)); _eo_typeco (o) = _ERB_CO_STR   ; } return o; }
eobj erb_addO( erb t, ekey key, eobj   obj) { is1_ret(!t || !obj || (_eo_keys(t) && !_k_keyS(key)), 0); return __erb_addO(t, key, obj, 0); }

eobj erb_addMI(erb t, ekey key, i64    val) { _erbn_t b = {{0}, key, {._len =                         8, ._typ = {.t_coe = _ERB_COE_NUM_I}}, {.i =       val} }; eobj o = __erb_makeRoom(t, _n_o(&b), 1, 0); if(o) {              _eo_setI (o, val            ); _eo_typecoe(o) = _ERB_COE_NUM_I; } return o; }
eobj erb_addMF(erb t, ekey key, f64    val) { _erbn_t b = {{0}, key, {._len =                         8, ._typ = {.t_coe = _ERB_COE_NUM_F}}, {.f =       val} }; eobj o = __erb_makeRoom(t, _n_o(&b), 1, 0); if(o) {              _eo_setF (o, val            ); _eo_typecoe(o) = _ERB_COE_NUM_F; } return o; }
eobj erb_addMP(erb t, ekey key, conptr ptr) { _erbn_t b = {{0}, key, {._len =               sizeof(ptr), ._typ = {.t_coe = _ERB_COE_PTR  }}, {.p = (cptr)ptr} }; eobj o = __erb_makeRoom(t, _n_o(&b), 1, 0); if(o) {              _eo_setP (o, ptr            ); _eo_typeco (o) = _ERB_CO_PTR   ; } return o; }
eobj erb_addMR(erb t, ekey key, size   len) { _erbn_t b = {{0}, key, {._len =                   len + 1, ._typ = {.t_coe = _ERB_COE_RAW  }}, {             0} }; eobj o = __erb_makeRoom(t, _n_o(&b), 1, 0); if(o) {              _eo_wipeR(o, len            ); _eo_typeco (o) = _ERB_CO_RAW   ; } return o; }
eobj erb_addMS(erb t, ekey key, constr str) { _erbn_t b = {{0}, key, {._len = str ? strlen(str) + 1 : 1, ._typ = {.t_coe = _ERB_COE_STR  }}, {.s = (cstr)str} }; eobj o = __erb_makeRoom(t, _n_o(&b), 1, 0); if(o) {_n_len(&b)--; _eo_setS (o, str, _n_len(&b)); _eo_typeco (o) = _ERB_CO_STR   ; } return o; }
eobj erb_addMO(erb t, ekey key, eobj   obj) { is1_ret(!t || !obj || (_eo_keys(t) && !_k_keyS(key)), 0); return __erb_addO(t, key, obj, 1); }

eobj erb_setI( erb t, ekey key, i64    val) { _erbn_t b = {{0}, key, {._len =                         8, ._typ = {.t_coe = _ERB_COE_NUM_I}}, {.i =       val} }; eobj o = __erb_makeRoom(t, _n_o(&b), 0, 1); if(o) {               _eo_setI (o, val            ); _eo_typecoe(o) = _ERB_COE_NUM_I; } return o; }
eobj erb_setF( erb t, ekey key, f64    val) { _erbn_t b = {{0}, key, {._len =                         8, ._typ = {.t_coe = _ERB_COE_NUM_F}}, {.f =       val} }; eobj o = __erb_makeRoom(t, _n_o(&b), 0, 1); if(o) {               _eo_setF (o, val            ); _eo_typecoe(o) = _ERB_COE_NUM_F; } return o; }
eobj erb_setP( erb t, ekey key, conptr ptr) { _erbn_t b = {{0}, key, {._len =               sizeof(ptr), ._typ = {.t_coe = _ERB_COE_PTR  }}, {.p = (cptr)ptr} }; eobj o = __erb_makeRoom(t, _n_o(&b), 0, 1); if(o) {               _eo_setP (o, ptr            ); _eo_typeco (o) = _ERB_CO_PTR   ; } return o; }
eobj erb_setR( erb t, ekey key, u32    len) { _erbn_t b = {{0}, key, {._len =                   len + 1, ._typ = {.t_coe = _ERB_COE_RAW  }}, {             0} }; eobj o = __erb_makeRoom(t, _n_o(&b), 0, 1); if(o) {               _eo_wipeR(o, len            ); _eo_typeco (o) = _ERB_CO_RAW   ; } return o; }
eobj erb_setS( erb t, ekey key, constr str) { _erbn_t b = {{0}, key, {._len = str ? strlen(str) + 1 : 1, ._typ = {.t_coe = _ERB_COE_STR  }}, {             0} }; eobj o = __erb_makeRoom(t, _n_o(&b), 0, 1); if(o) { _n_len(&b)--; _eo_setS (o, str, _n_len(&b)); _eo_typeco (o) = _ERB_CO_STR   ; } return o; }

inline uint  erb_lenO(cptr obj) { return obj ? _eo_len(obj) : 0;}


eobj erb_find(erb t, ekey key)
{
    rb_node *itr; i64 ret;

    is0_ret(t, 0);

    itr = _c_rn(t);

    if(_eo_keys(t))
    {
        is0_ret(_k_keyS(key), 0);

        while(itr)
        {
            ret = strcmp(_k_keyS(key), _l_keyS(itr));

            if     (ret < 0) itr = itr->rb_left;
            else if(ret > 0) itr = itr->rb_right;
            else             return _l_o(itr);

        }
    }
    else
    {
        while(itr)
        {
            ret = _k_keyI(key) - _l_keyI(itr);

            if     (ret < 0) itr = itr->rb_left;
            else if(ret > 0) itr = itr->rb_right;
            else             return _l_o(itr);

        }
    }

    return NULL;
}

eobj erb_val (erb t, ekey key) { return erb_find(t, key); }
i64  erb_valI(erb t, ekey key) { eobj o = erb_find(t, key); _eo_retI(o); }
f64  erb_valF(erb t, ekey key) { eobj o = erb_find(t, key); _eo_retF(o); }
cptr erb_valP(erb t, ekey key) { eobj o = erb_find(t, key); _eo_retP(o); }
cstr erb_valS(erb t, ekey key) { eobj o = erb_find(t, key); _eo_retS(o); }
cptr erb_valR(erb t, ekey key) { eobj o = erb_find(t, key); _eo_retR(o); }

etypeo erb_valType  (erb t, ekey key) { eobj o = erb_find(t, key); _eo_retT(o); }
uint   erb_valLen   (erb t, ekey key) { eobj o = erb_find(t, key); _eo_retL(o); }
bool   erb_valIsTrue(erb r, ekey key) { return __eobj_isTrue(erb_find(r, key)); }

eobj erb_first(erb  t) { if(t) { rb_node* l = rb_first(&_c_rb(t)); if(l) return _l_o(l);} return 0;}
eobj erb_last (erb  t) { if(t) { rb_node* l = rb_last (&_c_rb(t)); if(l) return _l_o(l);} return 0;}
eobj erb_next (eobj o) { if(o) { rb_node* l = rb_next (&_eo_l(o)); if(l) return _l_o(l);} return 0;}
eobj erb_prev (eobj o) { if(o) { rb_node* l = rb_prev (&_eo_l(o)); if(l) return _l_o(l);} return 0;}

eobj erb_takeH(erb  t) { eobj o = erb_first(t); if(o){ rb_erase(&_eo_l(o), &_c_rb(t)); _c_len(t)--;} return o; }
eobj erb_takeT(erb  t) { eobj o = erb_last (t); if(o){ rb_erase(&_eo_l(o), &_c_rb(t)); _c_len(t)--;} return o; }

eobj erb_takeO  (erb t, eobj obj) { is1_ret(!t || !obj || !_eo_linked(obj) || _eo_typeo(obj) != ERB, 0); rb_erase(&_eo_l(obj), &_c_rb(t)); _c_len(t)--; return obj; }
eobj erb_takeOne(erb t, ekey key) { eobj o = erb_val(t, key); if(o){ rb_erase(&_eo_l(o), &_c_rb(t)); _c_len(t)--; } return o; }

int  erb_freeH(erb t) { eobj o = erb_takeH(t); if(o) { if(_c_rls(t)) _c_rls(t)(o, _c_prvt(t)); if(_eo_keys(o)) _eo_freeK(o); _eo_free(o); return 1; } return 0; }
int  erb_freeT(erb t) { eobj o = erb_takeT(t); if(o) { if(_c_rls(t)) _c_rls(t)(o, _c_prvt(t)); if(_eo_keys(o)) _eo_freeK(o); _eo_free(o); return 1; } return 0; }

int  erb_freeO  (erb t, eobj   o) {      o = erb_takeO  (t,   o); if(o) { if(_eo_keys(t)) _eo_freeK(o); efree(&_eo_l(o)); return 1; } return 0; }
int  erb_freeOne(erb t, ekey key) { eobj o = erb_takeOne(t, key); if(o) { if(_eo_keys(t)) _eo_freeK(o); efree(&_eo_l(o)); return 1; } return 0; }

void erb_show (erb t, uint len)
{
    rb_node* itr;

    is0_ret(t, );

    if (len > _c_len(t)) len = _c_len(t);

    printf("(erb: %s %d/%d)", _eo_keys(t) ? "STR" : "INT", len, _c_len(t));

    is0_exeret(len > 0, puts("");fflush(stdout);, );

    printf(": {\n");

    if(_eo_keys(t))
    {
        for(itr = rb_first(&_c_rb(t)); itr; itr = rb_next(itr))
        {
            switch (_l_typeoe(itr)) {
            case _EFALSE: printf("    \"%s\": false,\n"          , _l_keyS(itr));break;
            case _ETRUE : printf("    \"%s\": true,\n"           , _l_keyS(itr));break;
            case _ENULL : printf("    \"%s\": null,\n"           , _l_keyS(itr));break;
            case _ENUM_I: printf("    \"%s\": \"%"PRIi64"\",\n"  , _l_keyS(itr), _l_valI(itr)); break;
            case _ENUM_F: printf("    \"%s\": \"%f\",\n"         , _l_keyS(itr), _l_valF(itr)); break;
            case _EPTR  : printf("    \"%s\": \"%p\",\n"         , _l_keyS(itr), _l_valP(itr)); break;
            case _ESTR  : printf("    \"%s\": \"%s\",\n"         , _l_keyS(itr), _l_valS(itr)); break;
            case _ERAW  : printf("    \"%s\": RAW(...),\n"       , _l_keyS(itr));break;
            case _EOBJ  : printf("    \"%s\": OBJ,\n"            , _l_keyS(itr));break;
            }
            if(--len == 0) break;
        }
    }
    else
    {
        for(itr = rb_first(&_c_rb(t)); itr; itr = rb_next (itr))
        {
            switch (_l_typeoe(itr)) {
            case _EFALSE     : printf("    \"%"PRIi64"\": false,\n"         , _l_keyI(itr)); break;
            case _ETRUE      : printf("    \"%"PRIi64"\": true,\n"          , _l_keyI(itr)); break;
            case _ENULL      : printf("    \"%"PRIi64"\": null,\n"          , _l_keyI(itr)); break;
            case _ENUM_I     : printf("    \"%"PRIi64"\": %"PRIi64",\n"     , _l_keyI(itr), _l_valI(itr)); break;
            case _ENUM_F     : printf("    \"%"PRIi64"\": %f,\n"            , _l_keyI(itr), _l_valF(itr)); break;
            case _EPTR       : printf("    \"%"PRIi64"\": %p,\n"            , _l_keyI(itr), _l_valP(itr)); break;
            case _ESTR       : printf("    \"%"PRIi64"\": \"%s\",\n"        , _l_keyI(itr), _l_valS(itr)); break;
            case _ERAW       : printf("    \"%"PRIi64"\": RAW(...),\n"      , _l_keyI(itr));break;
            }
            if(--len == 0) break;
        }
    }

    printf("}\n"); fflush(stdout);
}

