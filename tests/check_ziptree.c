/* This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at http://mozilla.org/MPL/2.0/. */

#include <check.h>
#include <stdio.h>
#include <stdlib.h>
#include "ziptree.h"
#include "assert.h"

struct treeEntry {
    unsigned int key;
    ZIP_ENTRY(treeEntry) pointers;
};

ZIP_HEAD(tree, treeEntry);

static enum ZIP_CMP
compareKeys(const void *k1, const void *k2) {
    unsigned int key1 = *(const unsigned int*)k1;
    unsigned int key2 = *(const unsigned int*)k2;
    if(key1 == key2)
        return ZIP_CMP_EQ;
    return (key1 < key2) ? ZIP_CMP_LESS : ZIP_CMP_MORE;
}

ZIP_FUNCTIONS(tree, treeEntry, pointers, unsigned int, key, compareKeys)

static void
checkTree(struct treeEntry *e) {
    if(!e)
        return;

    struct treeEntry *left = e->pointers.left;
    if(left)
        ck_assert_uint_le(left->key, e->key);

    struct treeEntry *right = e->pointers.right;
    if(right)
        ck_assert_uint_ge(right->key, e->key);

    checkTree(left);
    checkTree(right);
}

START_TEST(randTree) {
    srand(0);
    struct tree t1 = {NULL};
    for(unsigned int i = 0; i < 100; i++) {
        struct treeEntry *e1 = (struct treeEntry*)malloc(sizeof(struct treeEntry));
        e1->key = (unsigned int)rand();
        ZIP_INSERT(tree, &t1, e1);
    }

    checkTree(t1.root);

    while(t1.root) {
        checkTree(t1.root);
        struct treeEntry *left = t1.root->pointers.left;
        struct treeEntry *right = t1.root->pointers.right;
        free(t1.root);
        t1.root = ZIP_ZIP(tree, left, right);
    }
} END_TEST

START_TEST(mergeTrees) {
    struct tree t1 = {NULL};
    struct tree t2 = {NULL};
    for(unsigned int i = 0; i < 100; i++) {
        struct treeEntry *e1 = (struct treeEntry*)malloc(sizeof(struct treeEntry));
        struct treeEntry *e2 = (struct treeEntry*)malloc(sizeof(struct treeEntry));
        e1->key = i;
        e2->key = i + 1000;
        ZIP_INSERT(tree, &t1, e1);
        ZIP_INSERT(tree, &t2, e2);
    }

    checkTree(t1.root);
    checkTree(t2.root);

    struct tree t3;
    t3.root = ZIP_ZIP(tree, t1.root, t2.root);

    checkTree(t3.root);

    while(t3.root) {
        checkTree(t3.root);
        struct treeEntry *left = t3.root->pointers.left;
        struct treeEntry *right = t3.root->pointers.right;
        free(t3.root);
        t3.root = ZIP_ZIP(tree, left, right);
    }
} END_TEST

START_TEST(splitTree) {
    struct tree t1 = {NULL};
    for(unsigned int i = 0; i < 100; i++) {
        struct treeEntry *e1 = (struct treeEntry*)malloc(sizeof(struct treeEntry));
        struct treeEntry *e2 = (struct treeEntry*)malloc(sizeof(struct treeEntry));
        e1->key = i;
        e2->key = i;
        ZIP_INSERT(tree, &t1, e1);
        ZIP_INSERT(tree, &t1, e2);
    }

    checkTree(t1.root);

    for(unsigned int split_key = 50; split_key < 60; split_key++) {
        struct tree t2;
        struct tree t3;
        ZIP_UNZIP(tree, &t1, &split_key, &t2, &t3);
        checkTree(t2.root);
        checkTree(t3.root);
        t1.root = ZIP_ZIP(tree, t2.root, t3.root);
        checkTree(t1.root);
    }

    while(t1.root) {
        checkTree(t1.root);
        struct treeEntry *left = t1.root->pointers.left;
        struct treeEntry *right = t1.root->pointers.right;
        free(t1.root);
        t1.root = ZIP_ZIP(tree, left, right);
    }
} END_TEST

int main(void) {
    int number_failed = 0;
    TCase *tc_parse = tcase_create("ziptree");
    tcase_add_test(tc_parse, randTree);
    tcase_add_test(tc_parse, mergeTrees);
    tcase_add_test(tc_parse, splitTree);
    Suite *s = suite_create("Test ziptree library");
    suite_add_tcase(s, tc_parse);
    SRunner *sr = srunner_create(s);
    srunner_set_fork_status(sr, CK_NOFORK);
    srunner_run_all(sr, CK_NORMAL);
    number_failed += srunner_ntests_failed(sr);
    srunner_free(sr);
    return (number_failed == 0) ? EXIT_SUCCESS : EXIT_FAILURE;
}

