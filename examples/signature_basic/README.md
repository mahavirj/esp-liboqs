# ML-DSA Signature Basic Example

This example demonstrates the basic usage of post-quantum signature algorithms using the esp-liboqs component.

## What This Example Does

1. Initializes liboqs with ESP32 hardware RNG (automatic)
2. Tests ML-DSA-65 (NIST standardized signature algorithm)
3. Generates a keypair
4. Signs a test message
5. Verifies the signature (positive test)
6. Verifies a corrupted signature fails (negative test)
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
I (XXX) sig_example: ========================================
I (XXX) sig_example: liboqs Signature Example for ESP-IDF
I (XXX) sig_example: ========================================
I (XXX) sig_example: liboqs version: 0.15.0
I (XXX) sig_example: Chip: esp32
I (XXX) sig_example: Initial free heap: XXXXX bytes (XXX.X KB)
I (XXX) sig_example: Total signature algorithms available: X
I (XXX) sig_example: ========================================
I (XXX) sig_example: Testing signature: ML-DSA-65
I (XXX) sig_example: ========================================
I (XXX) sig_example: Public key:    1952 bytes
I (XXX) sig_example: Secret key:    4032 bytes
I (XXX) sig_example: Signature:     3293 bytes
I (XXX) sig_example: Step 1: Generating keypair...
I (XXX) sig_example:   Keypair generated in XX ms
I (XXX) sig_example: Step 2: Signing message...
I (XXX) sig_example:   Signed in XX ms
I (XXX) sig_example:   Signature length: 3293 bytes
I (XXX) sig_example: Step 3: Verifying valid signature...
I (XXX) sig_example:   ✓ SUCCESS: Signature is valid!
I (XXX) sig_example:   Verified in XX ms
I (XXX) sig_example: Step 4: Verifying corrupted signature (should fail)...
I (XXX) sig_example:   ✓ SUCCESS: Corrupted signature correctly rejected!
I (XXX) sig_example: Performance summary:
I (XXX) sig_example:   Keypair:    XX ms
I (XXX) sig_example:   Sign:       XX ms
I (XXX) sig_example:   Verify:     XX ms
I (XXX) sig_example:   Total:      XX ms
```

## Configuration

You can enable additional signature algorithms via menuconfig:

```bash
idf.py menuconfig
# Navigate to: Component config → liboqs Post-Quantum Cryptography → Algorithm Selection
```

## Available Signature Algorithms

- **ML-DSA (FIPS 204)**: ML-DSA-44, ML-DSA-65, ML-DSA-87 (NIST standardized)
- **Falcon**: Falcon-512, Falcon-1024
- **SPHINCS+**: Various parameter sets (currently disabled due to compilation issue)
- **MAYO**: MAYO-1, MAYO-2, MAYO-3
- **CROSS**: Various parameter sets
- **SNOVA**: Various parameter sets
- **UOV**: UOV-Ip, UOV-Is, UOV-III

## Memory Requirements

- Stack: 16 KB (configured in sdkconfig.defaults)
- Heap: Varies by algorithm
  - ML-DSA-44: ~8 KB
  - ML-DSA-65: ~10 KB
  - ML-DSA-87: ~14 KB
  - Falcon-512: ~10 KB
  - Falcon-1024: ~15 KB
- Flash: Approximately 150-250 KB depending on enabled algorithms

## Supported Targets

All ESP32 variants:
- ESP32 (Xtensa)
- ESP32-S2 (Xtensa)
- ESP32-S3 (Xtensa)
- ESP32-C2 (RISC-V) - Use ML-DSA-44 for lower memory
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
1. Using smaller algorithm variants (ML-DSA-44 instead of ML-DSA-87)
2. Increasing heap size if PSRAM is available
3. Reducing stack size if not using FALCON

### RNG Not Initialized Error

If you see "OQS_randombytes_system is not available", make sure:
- CONFIG_LIBOQS_AUTO_INIT_RNG=y in sdkconfig
- Or manually call `esp_liboqs_rng_init()` before using liboqs

## Algorithm Comparison

| Algorithm | Public Key | Secret Key | Signature | Security Level | Speed |
|-----------|------------|------------|-----------|----------------|-------|
| ML-DSA-44 | 1312 B | 2560 B | 2420 B | NIST Level 2 | Fast |
| ML-DSA-65 | 1952 B | 4032 B | 3293 B | NIST Level 3 | Medium |
| ML-DSA-87 | 2592 B | 4896 B | 4627 B | NIST Level 5 | Slower |
| Falcon-512 | 897 B | 1281 B | ~690 B | NIST Level 1 | Fast |
| Falcon-1024 | 1793 B | 2305 B | ~1330 B | NIST Level 5 | Medium |

## Understanding the Test

This example performs a complete signature workflow:

1. **Keypair Generation**: Creates a public/private key pair
2. **Signing**: Uses the private key to sign a random test message
3. **Verification (Positive)**: Uses the public key to verify the signature is valid
4. **Verification (Negative)**: Corrupts the signature and confirms verification fails

This demonstrates both the correct operation and security properties of the signature scheme.
