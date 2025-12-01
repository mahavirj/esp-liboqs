# ML-KEM Basic Example

This example demonstrates the basic usage of the ML-KEM (Key Encapsulation Mechanism) algorithm using the esp-liboqs component.

## What This Example Does

1. Initializes liboqs with ESP32 hardware RNG (automatic)
2. Tests ML-KEM-768 (NIST standardized algorithm)
3. Generates a keypair
4. Performs encapsulation (sender side)
5. Performs decapsulation (receiver side)
6. Verifies the shared secrets match
7. Reports performance metrics

## Building and Running

### Configure the Target

```bash
idf.py set-target esp32  # or esp32s3, esp32c3, etc.
```

### Build

```bash
idf.py build
```

### Flash and Monitor

```bash
idf.py flash monitor
```

## Expected Output

```
I (XXX) kem_example: ========================================
I (XXX) kem_example: liboqs KEM Example for ESP-IDF
I (XXX) kem_example: ========================================
I (XXX) kem_example: liboqs version: 0.15.0
I (XXX) kem_example: Chip: esp32
I (XXX) kem_example: Initial free heap: XXXXX bytes (XXX.X KB)
I (XXX) kem_example: Total KEM algorithms available: X
I (XXX) kem_example: ========================================
I (XXX) kem_example: Testing KEM: ML-KEM-768
I (XXX) kem_example: ========================================
I (XXX) kem_example: Public key:    1184 bytes
I (XXX) kem_example: Secret key:    2400 bytes
I (XXX) kem_example: Ciphertext:    1088 bytes
I (XXX) kem_example: Shared secret: 32 bytes
I (XXX) kem_example: Step 1: Generating keypair...
I (XXX) kem_example:   Keypair generated in XX ms
I (XXX) kem_example: Step 2: Encapsulating shared secret...
I (XXX) kem_example:   Encapsulated in XX ms
I (XXX) kem_example: Step 3: Decapsulating shared secret...
I (XXX) kem_example:   Decapsulated in XX ms
I (XXX) kem_example: Step 4: Verifying shared secrets...
I (XXX) kem_example:   ✓ SUCCESS: Shared secrets match!
I (XXX) kem_example: Performance summary:
I (XXX) kem_example:   Keypair:    XX ms
I (XXX) kem_example:   Encaps:     XX ms
I (XXX) kem_example:   Decaps:     XX ms
I (XXX) kem_example:   Total:      XX ms
```

## Configuration

You can enable additional algorithms via menuconfig:

```bash
idf.py menuconfig
# Navigate to: Component config → liboqs Post-Quantum Cryptography → Algorithm Selection
```

## Memory Requirements

- Stack: 20 KB (configured in sdkconfig.defaults)
- Heap: Varies by algorithm, ML-KEM-768 uses ~8-12 KB during operation
- Flash: Approximately 100-200 KB for ML-KEM-768

## Supported Targets

All ESP32 variants:
- ESP32 (Xtensa)
- ESP32-S2 (Xtensa)
- ESP32-S3 (Xtensa)
- ESP32-C2 (RISC-V) - Use ML-KEM-512 for lower memory
- ESP32-C3 (RISC-V)
- ESP32-C6 (RISC-V)
- ESP32-H2 (RISC-V)
- ESP32-P4 (RISC-V)

## Troubleshooting

### Build Fails with "liboqs not found"

Make sure you've initialized the git submodule:
```bash
cd ../..
git submodule update --init --recursive
```

### Out of Memory Errors

Reduce memory usage by:
1. Using smaller algorithm variants (ML-KEM-512 instead of ML-KEM-1024)
2. Increasing heap size if PSRAM is available
3. Reducing stack size if not using FALCON

### RNG Not Initialized Error

If you see "OQS_randombytes_system is not available", make sure:
- CONFIG_LIBOQS_AUTO_INIT_RNG=y in sdkconfig
- Or manually call `esp_liboqs_rng_init()` before using liboqs
