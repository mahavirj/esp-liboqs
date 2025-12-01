# liboqs for ESP32

Post-quantum cryptography library for ESP32 microcontrollers, based on the Open Quantum Safe [liboqs](https://github.com/open-quantum-safe/liboqs) project.

## Features

- **NIST-standardized algorithms**: ML-KEM (FIPS 203), ML-DSA (FIPS 204), SLH-DSA (FIPS 205)
- **Additional algorithms**: FALCON, SPHINCS+, MAYO, CROSS, SNOVA
- **Hardware RNG integration**: Uses ESP32's hardware random number generator
- **Automatic initialization**: RNG callback configured automatically at startup
- **Menuconfig integration**: Easy algorithm selection via `idf.py menuconfig`
- **Multi-architecture support**: Xtensa (ESP32/S2/S3) and RISC-V (ESP32-C2/C3/C6/H2/P4)
- **Memory-optimized**: Profiles for constrained and generous memory configurations
- **Examples included**: KEM and signature demonstrations

## Quick Start

### Installation

#### Via ESP Component Registry (recommended)

Add to your project's `idf_component.yml`:

```yaml
dependencies:
  esp-liboqs:
    version: "^0.15.0"
```

#### Manual Installation

```bash
cd components
git clone --recursive https://github.com/espressif/esp-liboqs.git
```

### Basic Usage

```c
#include <oqs/oqs.h>
#include "esp_log.h"

void app_main(void)
{
    ESP_LOGI("app", "liboqs version: %s", OQS_version());

    // RNG is auto-initialized if CONFIG_LIBOQS_AUTO_INIT_RNG=y

    // Test ML-KEM-768
    OQS_KEM *kem = OQS_KEM_new("ML-KEM-768");

    uint8_t *public_key = malloc(kem->length_public_key);
    uint8_t *secret_key = malloc(kem->length_secret_key);

    OQS_KEM_keypair(kem, public_key, secret_key);

    // ... use keys ...

    free(public_key);
    OQS_MEM_secure_free(secret_key, kem->length_secret_key);
    OQS_KEM_free(kem);
}
```

### Configuration

Enable algorithms via menuconfig:

```bash
idf.py menuconfig
# Navigate to: Component config → liboqs Post-Quantum Cryptography
```

Default enabled algorithms:
- ML-KEM (Key Encapsulation)
- ML-DSA (Digital Signatures)
- FALCON, SPHINCS+, MAYO, CROSS, SNOVA (Signatures)

## Performance Benchmarks

Performance measured on ESP32-C5 @ 240 MHz (RISC-V):

### ML-KEM (Key Encapsulation)

| Algorithm | Keypair | Encaps | Decaps | Stack Used | Heap Used |
|-----------|---------|--------|--------|------------|-----------|
| ML-KEM-512 | ~95 ms | ~95 ms | ~115 ms | ~14 KB | ~4 KB |
| ML-KEM-768 | ~148 ms | ~148 ms | ~178 ms | ~17 KB | ~6 KB |
| ML-KEM-1024 | ~230 ms | ~230 ms | ~275 ms | ~21 KB | ~8 KB |

### ML-DSA (Digital Signatures)

| Algorithm | Keypair | Sign | Verify | Stack Used | Heap Used |
|-----------|---------|------|--------|------------|-----------|
| ML-DSA-44 | ~395 ms | ~415 ms | ~370 ms | ~55 KB | ~9 KB |
| ML-DSA-65 | ~623 ms | ~625 ms | ~581 ms | ~78 KB | ~13 KB |
| ML-DSA-87 | ~975 ms | ~1025 ms | ~915 ms | ~110 KB | ~18 KB |

*Note: Performance varies by chip architecture and clock speed.

## Examples

See the `examples/` directory for complete examples:

- `kem_basic`: Basic key encapsulation mechanism demonstration
- `signature_basic`: Digital signature demonstration

## ESP-IDF Compatibility

- **Minimum**: ESP-IDF v4.4
- **Recommended**: ESP-IDF v5.0+
- **Tested**: ESP-IDF v6.1-dev

## Architecture Notes

### Xtensa Support

ESP32, ESP32-S2, and ESP32-S3 use the Xtensa architecture, which is not officially supported by liboqs. This port enables compilation via `OQS_PERMIT_UNSUPPORTED_ARCHITECTURE`, using reference implementations (no SIMD optimizations). These implementations are secure but slower than optimized versions.

### RISC-V Support

ESP32-C2, ESP32-C3, ESP32-C6, ESP32-H2, and ESP32-P4 use RISC-V architecture with better support for optimized implementations.

## Security Considerations

- **RNG**: This component automatically initializes liboqs to use ESP-IDF's hardware RNG (`esp_fill_random()`), which is cryptographically secure.
- **Production use**: While liboqs implements NIST-standardized algorithms, always perform your own security assessment for production deployments.
- **Memory safety**: Use `OQS_MEM_secure_free()` for sensitive data (private keys, shared secrets).

## Contributing

Contributions are welcome! Please:

1. Fork the repository
2. Create a feature branch
3. Test on multiple ESP32 variants
4. Submit a pull request

## License

MIT License - See [LICENSE](LICENSE) file for details.

This component is based on [liboqs](https://github.com/open-quantum-safe/liboqs) by the Open Quantum Safe project.
