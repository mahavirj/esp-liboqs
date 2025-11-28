// SPDX-License-Identifier: MIT
//
// Basic ML-DSA Signature example for ESP-IDF
// Based on liboqs/zephyr/samples/Signatures/src/main.c

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include <oqs/oqs.h>

static const char *TAG = "sig_example";

/**
 * @brief Print first 32 bytes of data in hex format
 */
static void print_hex(const char *label, const uint8_t *data, size_t len)
{
    printf("%-20s (%4zu bytes): ", label, len);
    size_t print_len = (len > 32) ? 32 : len;
    for (size_t i = 0; i < print_len; i++) {
        printf("%02X", data[i]);
    }
    if (len > 32) {
        printf("...");
    }
    printf("\n");
}

/**
 * @brief Test a signature algorithm
 */
static void test_signature(const char *alg_name)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Testing signature: %s", alg_name);
    ESP_LOGI(TAG, "========================================");

    // Create signature instance
    OQS_SIG *sig = OQS_SIG_new(alg_name);
    if (sig == NULL) {
        ESP_LOGE(TAG, "Failed to create signature instance for %s", alg_name);
        return;
    }

    // Log sizes
    ESP_LOGI(TAG, "Public key:    %zu bytes", sig->length_public_key);
    ESP_LOGI(TAG, "Secret key:    %zu bytes", sig->length_secret_key);
    ESP_LOGI(TAG, "Signature:     %zu bytes", sig->length_signature);

    // Define message length
    const size_t message_len = 100;

    // Allocate buffers
    uint8_t *public_key = malloc(sig->length_public_key);
    uint8_t *secret_key = malloc(sig->length_secret_key);
    uint8_t *message = malloc(message_len);
    uint8_t *signature = malloc(sig->length_signature);

    if (!public_key || !secret_key || !message || !signature) {
        ESP_LOGE(TAG, "Memory allocation failed!");
        goto cleanup;
    }

    // Step 1: Generate keypair
    ESP_LOGI(TAG, "Step 1: Generating keypair...");
    uint32_t start = esp_log_timestamp();
    if (OQS_SIG_keypair(sig, public_key, secret_key) != OQS_SUCCESS) {
        ESP_LOGE(TAG, "Keypair generation failed");
        goto cleanup;
    }
    uint32_t keypair_time = esp_log_timestamp() - start;
    ESP_LOGI(TAG, "  Keypair generated in %lu ms", keypair_time);
    print_hex("  Public key", public_key, sig->length_public_key);

    // Generate random test message
    OQS_randombytes(message, message_len);
    print_hex("  Test message", message, message_len);

    // Step 2: Sign message
    ESP_LOGI(TAG, "Step 2: Signing message...");
    start = esp_log_timestamp();
    size_t signature_len;
    if (OQS_SIG_sign(sig, signature, &signature_len, message, message_len, secret_key) != OQS_SUCCESS) {
        ESP_LOGE(TAG, "Signing failed");
        goto cleanup;
    }
    uint32_t sign_time = esp_log_timestamp() - start;
    ESP_LOGI(TAG, "  Signed in %lu ms", sign_time);
    ESP_LOGI(TAG, "  Signature length: %zu bytes", signature_len);
    print_hex("  Signature", signature, signature_len);

    // Step 3: Verify signature (should succeed)
    ESP_LOGI(TAG, "Step 3: Verifying valid signature...");
    start = esp_log_timestamp();
    if (OQS_SIG_verify(sig, message, message_len, signature, signature_len, public_key) != OQS_SUCCESS) {
        ESP_LOGE(TAG, "  ✗ FAILURE: Valid signature verification failed!");
        goto cleanup;
    }
    uint32_t verify_time = esp_log_timestamp() - start;
    ESP_LOGI(TAG, "  ✓ SUCCESS: Signature is valid!");
    ESP_LOGI(TAG, "  Verified in %lu ms", verify_time);

    // Step 4: Test negative case - corrupt signature and verify it fails
    ESP_LOGI(TAG, "Step 4: Verifying corrupted signature (should fail)...");

    // Save original signature for later
    uint8_t *original_signature = malloc(signature_len);
    if (original_signature) {
        memcpy(original_signature, signature, signature_len);
    }

    // Corrupt the signature
    OQS_randombytes(signature, signature_len);

    if (OQS_SIG_verify(sig, message, message_len, signature, signature_len, public_key) == OQS_SUCCESS) {
        ESP_LOGE(TAG, "  ✗ FAILURE: Corrupted signature should have been rejected!");
        if (original_signature) OQS_MEM_insecure_free(original_signature);
        goto cleanup;
    }
    ESP_LOGI(TAG, "  ✓ SUCCESS: Corrupted signature correctly rejected!");

    if (original_signature) {
        OQS_MEM_insecure_free(original_signature);
    }

    // Performance summary
    ESP_LOGI(TAG, "Performance summary:");
    ESP_LOGI(TAG, "  Keypair:    %lu ms", keypair_time);
    ESP_LOGI(TAG, "  Sign:       %lu ms", sign_time);
    ESP_LOGI(TAG, "  Verify:     %lu ms", verify_time);
    ESP_LOGI(TAG, "  Total:      %lu ms", keypair_time + sign_time + verify_time);

    uint32_t free_heap = esp_get_free_heap_size();
    ESP_LOGI(TAG, "Free heap: %lu bytes (%.1f KB)", free_heap, free_heap / 1024.0);

cleanup:
    // Securely free sensitive data
    if (secret_key) {
        OQS_MEM_secure_free(secret_key, sig->length_secret_key);
    }

    // Free non-sensitive data
    if (public_key) {
        OQS_MEM_insecure_free(public_key);
    }
    if (message) {
        OQS_MEM_insecure_free(message);
    }
    if (signature) {
        OQS_MEM_insecure_free(signature);
    }

    OQS_SIG_free(sig);
    ESP_LOGI(TAG, "");
}

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "liboqs Signature Example for ESP-IDF");
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "liboqs version: %s", OQS_version());
    ESP_LOGI(TAG, "Chip: %s", CONFIG_IDF_TARGET);
    ESP_LOGI(TAG, "");

    uint32_t initial_heap = esp_get_free_heap_size();
    ESP_LOGI(TAG, "Initial free heap: %lu bytes (%.1f KB)",
             initial_heap, initial_heap / 1024.0);
    ESP_LOGI(TAG, "");

    // Note: If CONFIG_LIBOQS_AUTO_INIT_RNG=y, RNG is already initialized
    // Otherwise, you must call esp_liboqs_rng_init() here

    ESP_LOGI(TAG, "Total signature algorithms available: %zu", OQS_SIG_alg_count());
    ESP_LOGI(TAG, "");

    // Test ML-DSA-65 (NIST standardized, balanced security/performance)
    if (OQS_SIG_alg_is_enabled("ML-DSA-65")) {
        test_signature("ML-DSA-65");
    } else {
        ESP_LOGW(TAG, "ML-DSA-65 not enabled in build");
        ESP_LOGW(TAG, "Enable via: idf.py menuconfig → Component config → liboqs");
    }

    // You can test other algorithms by uncommenting:
    /*
    if (OQS_SIG_alg_is_enabled("ML-DSA-44")) {
        test_signature("ML-DSA-44");
    }

    if (OQS_SIG_alg_is_enabled("ML-DSA-87")) {
        test_signature("ML-DSA-87");
    }

    if (OQS_SIG_alg_is_enabled("Falcon-512")) {
        test_signature("Falcon-512");
    }
    */

    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Example complete!");
    ESP_LOGI(TAG, "========================================");

    uint32_t final_heap = esp_get_free_heap_size();
    ESP_LOGI(TAG, "Final free heap: %lu bytes (%.1f KB)",
             final_heap, final_heap / 1024.0);

    if (final_heap < initial_heap) {
        ESP_LOGW(TAG, "Heap decreased by %lu bytes during execution",
                 initial_heap - final_heap);
    }
}
