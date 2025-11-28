// SPDX-License-Identifier: MIT
//
// Basic ML-KEM example for ESP-IDF
// Based on liboqs/zephyr/samples/KEMs/src/main.c

#include <stdio.h>
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_log.h"
#include "esp_system.h"
#include <oqs/oqs.h>

static const char *TAG = "kem_example";

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
 * @brief Test a KEM algorithm
 */
static void test_kem(const char *alg_name)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "Testing KEM: %s", alg_name);
    ESP_LOGI(TAG, "========================================");

    // Create KEM instance
    OQS_KEM *kem = OQS_KEM_new(alg_name);
    if (kem == NULL) {
        ESP_LOGE(TAG, "Failed to create KEM instance for %s", alg_name);
        return;
    }

    // Log sizes
    ESP_LOGI(TAG, "Public key:    %zu bytes", kem->length_public_key);
    ESP_LOGI(TAG, "Secret key:    %zu bytes", kem->length_secret_key);
    ESP_LOGI(TAG, "Ciphertext:    %zu bytes", kem->length_ciphertext);
    ESP_LOGI(TAG, "Shared secret: %zu bytes", kem->length_shared_secret);

    // Allocate buffers
    uint8_t *public_key = malloc(kem->length_public_key);
    uint8_t *secret_key = malloc(kem->length_secret_key);
    uint8_t *ciphertext = malloc(kem->length_ciphertext);
    uint8_t *shared_secret_e = malloc(kem->length_shared_secret);
    uint8_t *shared_secret_d = malloc(kem->length_shared_secret);

    if (!public_key || !secret_key || !ciphertext ||
        !shared_secret_e || !shared_secret_d) {
        ESP_LOGE(TAG, "Memory allocation failed!");
        goto cleanup;
    }

    // Step 1: Generate keypair
    ESP_LOGI(TAG, "Step 1: Generating keypair...");
    uint32_t start = esp_log_timestamp();
    if (OQS_KEM_keypair(kem, public_key, secret_key) != OQS_SUCCESS) {
        ESP_LOGE(TAG, "Keypair generation failed");
        goto cleanup;
    }
    uint32_t keypair_time = esp_log_timestamp() - start;
    ESP_LOGI(TAG, "  Keypair generated in %lu ms", keypair_time);
    print_hex("  Public key", public_key, kem->length_public_key);

    // Step 2: Encapsulation (sender side)
    ESP_LOGI(TAG, "Step 2: Encapsulating shared secret...");
    start = esp_log_timestamp();
    if (OQS_KEM_encaps(kem, ciphertext, shared_secret_e, public_key) != OQS_SUCCESS) {
        ESP_LOGE(TAG, "Encapsulation failed");
        goto cleanup;
    }
    uint32_t encaps_time = esp_log_timestamp() - start;
    ESP_LOGI(TAG, "  Encapsulated in %lu ms", encaps_time);
    print_hex("  Ciphertext", ciphertext, kem->length_ciphertext);
    print_hex("  Shared secret", shared_secret_e, kem->length_shared_secret);

    // Step 3: Decapsulation (receiver side)
    ESP_LOGI(TAG, "Step 3: Decapsulating shared secret...");
    start = esp_log_timestamp();
    if (OQS_KEM_decaps(kem, shared_secret_d, ciphertext, secret_key) != OQS_SUCCESS) {
        ESP_LOGE(TAG, "Decapsulation failed");
        goto cleanup;
    }
    uint32_t decaps_time = esp_log_timestamp() - start;
    ESP_LOGI(TAG, "  Decapsulated in %lu ms", decaps_time);

    // Step 4: Verify shared secrets match
    ESP_LOGI(TAG, "Step 4: Verifying shared secrets...");
    if (memcmp(shared_secret_e, shared_secret_d, kem->length_shared_secret) == 0) {
        ESP_LOGI(TAG, "  ✓ SUCCESS: Shared secrets match!");
    } else {
        ESP_LOGE(TAG, "  ✗ FAILURE: Shared secrets do not match!");
    }

    ESP_LOGI(TAG, "Performance summary:");
    ESP_LOGI(TAG, "  Keypair:    %lu ms", keypair_time);
    ESP_LOGI(TAG, "  Encaps:     %lu ms", encaps_time);
    ESP_LOGI(TAG, "  Decaps:     %lu ms", decaps_time);
    ESP_LOGI(TAG, "  Total:      %lu ms", keypair_time + encaps_time + decaps_time);

    uint32_t free_heap = esp_get_free_heap_size();
    ESP_LOGI(TAG, "Free heap: %lu bytes (%.1f KB)", free_heap, free_heap / 1024.0);
    ESP_LOGI(TAG, "Free Stack for main task: '%d'", uxTaskGetStackHighWaterMark(NULL));

cleanup:
    // Securely free sensitive data
    if (secret_key) {
        OQS_MEM_secure_free(secret_key, kem->length_secret_key);
    }
    if (shared_secret_e) {
        OQS_MEM_secure_free(shared_secret_e, kem->length_shared_secret);
    }
    if (shared_secret_d) {
        OQS_MEM_secure_free(shared_secret_d, kem->length_shared_secret);
    }

    // Free non-sensitive data
    if (public_key) {
        OQS_MEM_insecure_free(public_key);
    }
    if (ciphertext) {
        OQS_MEM_insecure_free(ciphertext);
    }

    OQS_KEM_free(kem);
    ESP_LOGI(TAG, "");
}

void app_main(void)
{
    ESP_LOGI(TAG, "========================================");
    ESP_LOGI(TAG, "liboqs KEM Example for ESP-IDF");
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

    ESP_LOGI(TAG, "Total KEM algorithms available: %zu", OQS_KEM_alg_count());
    ESP_LOGI(TAG, "");

    // Test ML-KEM-768 (most commonly used, NIST standardized)
    if (OQS_KEM_alg_is_enabled("ML-KEM-768")) {
        test_kem("ML-KEM-768");
    } else {
        ESP_LOGW(TAG, "ML-KEM-768 not enabled in build");
        ESP_LOGW(TAG, "Enable via: idf.py menuconfig → Component config → liboqs");
    }

    // You can test other algorithms by uncommenting:
    /*
    if (OQS_KEM_alg_is_enabled("ML-KEM-512")) {
        test_kem("ML-KEM-512");
    }

    if (OQS_KEM_alg_is_enabled("ML-KEM-1024")) {
        test_kem("ML-KEM-1024");
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
