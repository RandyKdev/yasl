#include "fortest.h"
#include "yats.h"

SETUP_YATS();

static void test_continue() {
	unsigned char expected[] = {
		0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x65, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		C_INT_1, 0,
		C_INT_1, 1,
		C_INT_1, 10,
		C_INT_1, 5,
		O_LIT, 0x00,
		O_BR_8,
		0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_LLOAD, 0x00,
		O_LIT, 0x01,
		O_ADD,
		O_LSTORE, 0x00,
		O_LLOAD, 0x00,
		O_LIT, 0x02,
		O_LT,
		O_BRF_8,
		0x23, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_LLOAD, 0x00,
		O_LIT, 0x03,
		O_EQ,
		O_BRF_8,
		0x09, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_BR_8,
		0xD4, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		O_LLOAD, 0x00,
		O_PRINT,
		O_BR_8,
		0xC8, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		O_POP,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "for let i = 0; i < 10; i += 1 { if i == 5 { continue; }; echo i; };");
}

static void test_break() {
	unsigned char expected[] = {
		0x20, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x66, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		0x04, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		C_INT_1, 0,
		C_INT_1, 1,
		C_INT_1, 10,
		C_INT_1, 5,
		O_LIT, 0x00,
		O_BR_8,
		0x07, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_LLOAD, 0x00,
		O_LIT, 0x01,
		O_ADD,
		O_LSTORE, 0x00,
		O_LLOAD, 0x00,
		O_LIT, 0x02,
		O_LT,
		O_BRF_8,
		0x24, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_LLOAD, 0x00,
		O_LIT, 0x03,
		O_EQ,
		O_BRF_8,
		0x0A, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,
		O_BCONST_F,
		O_BR_8,
		0xDF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		O_LLOAD, 0x00,
		O_PRINT,
		O_BR_8,
		0xC7, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF, 0xFF,
		O_POP,
		O_HALT
	};
	ASSERT_GEN_BC_EQ(expected, "for let i = 0; i < 10; i += 1 { if i == 5 { break; }; echo i; };");
}

int fortest(void) {
	test_continue();
	test_break();
	return __YASL_TESTS_FAILED__;
}
