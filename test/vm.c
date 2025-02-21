/*
 * kernel/test/vm.c
 * Virtual Memory Testing
 */

#include <rotary/test/vm.h>

/* ------------------------------------------------------------------------- */
/* Test Set-up and Clean-up                                                  */
/* ------------------------------------------------------------------------- */

int32_t vm_pre_module(ktest_module_t * module) {
    return 0;
}

/* ------------------------------------------------------------------------- */

int32_t vm_post_module(ktest_module_t * module) {
    return 0;
}

/* ------------------------------------------------------------------------- */

int32_t vm_pre_test(ktest_module_t * module) {
    return 0;
}

/* ------------------------------------------------------------------------- */

int32_t vm_post_test(ktest_module_t * module) {
    return 0;
}

/* ------------------------------------------------------------------------- */
/* Unit Tests                                                                */
/* ------------------------------------------------------------------------- */

void vm_test_space_new(ktest_unit_t * ktest) {
    struct vm_space * space = vm_space_new();
    assert_not_equal(space, NULL);
    assert_not_equal(space->pgd, NULL);
}

/* ------------------------------------------------------------------------- */

void vm_test_space_destroy(ktest_unit_t * ktest) {
    
}

/* ------------------------------------------------------------------------- */

void vm_test_space_add_map(ktest_unit_t * ktest) {

}

/* ------------------------------------------------------------------------- */

void vm_test_space_delete_map(ktest_unit_t * ktest) {

}

/* ------------------------------------------------------------------------- */

void vm_test_map_new(ktest_unit_t * ktest) {

}

/* ------------------------------------------------------------------------- */

void vm_test_map_destroy(ktest_unit_t * ktest) {

}

/* ------------------------------------------------------------------------- */
/* Test Registration                                                         */
/* ------------------------------------------------------------------------- */

static ktest_unit_t test_units[] = {
    KTEST_UNIT("vm-test-space-new", vm_test_space_new),
    KTEST_UNIT("vm-test-space-destroy", vm_test_space_destroy),
    KTEST_UNIT("vm-test-space-add-map", vm_test_space_add_map),
    KTEST_UNIT("vm-test-space-delete-map", vm_test_space_delete_map),
    KTEST_UNIT("vm-test-map-new", vm_test_map_new),
    KTEST_UNIT("vm-test-map-destroy", vm_test_map_destroy),
};

KTEST_MODULE_DEFINE("vm", test_units,
                    vm_pre_module,
                    vm_post_module,
                    vm_pre_test,
                    vm_post_test);

/* ------------------------------------------------------------------------- */

