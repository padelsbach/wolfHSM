#ifdef INCLUDE_WH_TEST_LIST

int test_flash_write_lock(void* ctx);
int test_flash_erase_program_verify(void* ctx);
int test_flash_unit_ops(void* ctx);
int test_nvm_add_overwrite_destroy(void* ctx);

int test_echo(whClientContext* ctx);
int test_server_info(whClientContext* ctx);
int whTest_CryptoSha256(whClientContext* ctx);
int whTestCrypto_Aes(whClientContext* ctx);
int whTestCrypto_Ecc256(whClientContext* ctx);

int test_cert_verify(whServerContext* ctx);

static const whTestCase whTests[] = {
    WH_TEST_MISC_TEST(test_flash_write_lock),
    WH_TEST_MISC_TEST(test_flash_erase_program_verify),
    WH_TEST_MISC_TEST(test_flash_unit_ops),
    WH_TEST_MISC_TEST(test_nvm_add_overwrite_destroy),

    WH_TEST_CLIENT_TEST(test_echo),
    WH_TEST_CLIENT_TEST(test_server_info),
    WH_TEST_CLIENT_TEST(whTest_CryptoSha256),
    WH_TEST_CLIENT_TEST(whTestCrypto_Aes),
    WH_TEST_CLIENT_TEST(whTestCrypto_Ecc256),

    WH_TEST_SERVER_TEST(test_cert_verify),
};

#endif /* INCLUDE_WH_TEST_LIST */
