# K9 Build and Development Guide

## Build System Overview

K9 uses Plan 9's `mk` build system, which provides clean dependency management and parallel builds across multiple architectures.

## Build Requirements

### Development Environment Options

1. **Plan 9 Native Environment** (Recommended)
   - Full Plan 9 installation with native toolchain
   - Cross-compilation support for all architectures

2. **Plan 9 from User Space (p9p)**
   - Linux/macOS compatibility layer
   - Install via package manager or from source

3. **Cross-Compilation Toolchains**
   - GCC cross-compilers for each target architecture
   - Specialized tools for cognitive algorithm optimization

### Required Tools

```bash
# For Plan 9 systems
mk          # Plan 9 make replacement
8c, 8l      # x86 compiler and linker  
5c, 5l      # ARM compiler and linker
q, q l      # PowerPC compiler and linker

# For cross-compilation
arm-linux-gnueabihf-gcc    # ARM cross-compiler
i686-linux-gnu-gcc         # x86 cross-compiler
powerpc-linux-gnu-gcc      # PowerPC cross-compiler
```

## Architecture-Specific Build Instructions

### x86 PC Architecture

```bash
cd k9/pc
mk clean && mk

# Build specific components
mk 9pc          # Kernel image
mk 9pcf         # Kernel with debugging
mk 9pccpu       # CPU server kernel
mk install      # Install to system
```

#### PC Configuration Files:
- `pc`: Basic kernel configuration
- `pcf`: Debug kernel with symbols
- `pccpu`: CPU server configuration
- `pcdisk`: Disk-based boot configuration

### ARM OMAP (BeagleBoard)

```bash
cd k9/omap
mk clean && mk

# Generate boot image
mk 9omap        # Kernel for SD card boot
mk install      # Install to SD card image
```

#### OMAP-Specific Features:
- TI OMAP3 SoC support
- DSP integration for cognitive acceleration
- Power management for mobile devices
- USB OTG support

### ARM BCM (Raspberry Pi)

```bash
cd k9/bcm  
mk clean && mk

# Build for specific Pi models
mk 9pi          # Raspberry Pi Model B
mk 9pi2         # Raspberry Pi 2 Model B
mk install      # Install to SD card
```

#### BCM-Specific Features:
- GPU acceleration via VideoCore IV
- GPIO interface for cognitive sensors
- Camera module support
- SPI/I2C for cognitive peripherals

### ARM Tegra2 (NVIDIA)

```bash
cd k9/teg2
mk clean && mk

# Build for Tegra development boards
mk 9tegra2      # Dual-core Cortex-A9 kernel
mk install      # Install to development board
```

#### Tegra2-Specific Features:
- Dual-core ARM Cortex-A9
- NVIDIA GPU acceleration
- Hardware video decode/encode
- High-speed I/O for cognitive data

### ARM Kirkwood (Marvell)

```bash
cd k9/kw
mk clean && mk

# Build for various Kirkwood boards
mk 9kw          # General Kirkwood kernel
mk install      # Install to target device
```

#### Kirkwood-Specific Features:
- Network acceleration engines
- Cryptographic acceleration
- SATA controller integration
- PCIe expansion for cognitive cards

### PowerPC

```bash
cd k9/ppc
mk clean && mk

# Build for PowerPC systems
mk 9ppc         # PowerPC G3/G4 kernel
mk install      # Install to PowerPC system
```

#### PowerPC-Specific Features:
- AltiVec SIMD acceleration
- Symmetric multiprocessing
- Large memory support (PAE)
- Scientific computing optimization

## Cognitive Algorithm Development

### Setting Up Cognitive Development Environment

1. **Create cognitive module directory**:
```bash
mkdir cognitive-module
cd cognitive-module
```

2. **Basic cognitive module structure**:
```c
// cogmod.c - Cognitive module template
#include "u.h"
#include "../port/lib.h"
#include "mem.h"
#include "dat.h"
#include "fns.h"

// Cognitive pattern structure
typedef struct CogPattern CogPattern;
struct CogPattern {
    ulong   id;
    ulong   attention;
    uchar   *pattern;
    ulong   size;
    CogPattern *next;
};

// Pattern recognition function
int
recognizepattern(CogPattern *pattern, uchar *input, ulong size)
{
    // Implement pattern matching algorithm
    // Return confidence score (0-100)
}

// Attention allocation function  
void
allocateattention(CogPattern *patterns, ulong available)
{
    // Distribute attention across patterns
    // Based on priority and current workload
}
```

3. **Integration with kernel**:
```c
// Add to appropriate device driver or kernel module
// Register cognitive functions with pattern engine
// Implement cognitive event handlers
```

### Hypergraph Data Structure Implementation

```c
// hypergraph.h
typedef struct HyperNode HyperNode;
typedef struct HyperEdge HyperEdge; 
typedef struct HyperGraph HyperGraph;

struct HyperNode {
    ulong       id;
    ulong       weight;
    void        *data;
    HyperEdge   **edges;
    ulong       nedges;
};

struct HyperEdge {
    ulong       id;
    ulong       weight;
    HyperNode   **nodes;
    ulong       nnodes;
    void        *relation;
};

struct HyperGraph {
    HyperNode   **nodes;
    HyperEdge   **edges;
    ulong       nnodes;
    ulong       nedges;
    Lock        lock;
};
```

### Neural-Symbolic Integration Example

```c
// neural_symbolic.c
typedef struct Symbol Symbol;
typedef struct NeuralNet NeuralNet;

struct Symbol {
    char    *name;
    ulong   type;
    void    *value;
    Symbol  *next;
};

struct NeuralNet {
    float   **weights;
    float   *biases;
    ulong   layers;
    ulong   *neurons;
};

// Bridge neural outputs to symbolic representation
Symbol*
neural_to_symbolic(NeuralNet *net, float *input)
{
    Symbol *symbols = nil;
    float *output;
    
    // Forward propagation through neural network
    output = neural_forward(net, input);
    
    // Convert neural activations to symbols
    for(int i = 0; i < net->neurons[net->layers-1]; i++) {
        if(output[i] > ACTIVATION_THRESHOLD) {
            Symbol *sym = allocsymbol();
            sym->name = symbolname(i);
            sym->value = &output[i];
            sym->next = symbols;
            symbols = sym;
        }
    }
    
    return symbols;
}
```

## Testing and Validation

### Unit Testing Framework

```c
// test_framework.c
typedef struct Test Test;
struct Test {
    char    *name;
    int     (*testfunc)(void);
    Test    *next;
};

static Test *tests = nil;

void
addtest(char *name, int (*func)(void))
{
    Test *t = malloc(sizeof(Test));
    t->name = name;
    t->testfunc = func;
    t->next = tests;
    tests = t;
}

int
runtests(void)
{
    Test *t;
    int passed = 0, failed = 0;
    
    for(t = tests; t != nil; t = t->next) {
        print("Running test: %s...", t->name);
        if(t->testfunc()) {
            print(" PASS\n");
            passed++;
        } else {
            print(" FAIL\n");
            failed++;
        }
    }
    
    print("Tests: %d passed, %d failed\n", passed, failed);
    return failed == 0;
}
```

### Cognitive Algorithm Testing

```c
// Test pattern recognition accuracy
int
test_pattern_recognition(void)
{
    CogPattern *pattern = createpattern();
    uchar testdata[] = {0x01, 0x02, 0x03, 0x04};
    
    int confidence = recognizepattern(pattern, testdata, sizeof(testdata));
    
    return confidence > 80; // 80% confidence threshold
}

// Test attention allocation fairness
int 
test_attention_allocation(void)
{
    CogPattern patterns[3];
    ulong total_attention = 1000;
    
    // Initialize test patterns
    patterns[0].attention = 0;
    patterns[1].attention = 0;
    patterns[2].attention = 0;
    
    allocateattention(patterns, total_attention);
    
    // Check that attention was distributed
    return (patterns[0].attention + patterns[1].attention + 
            patterns[2].attention) == total_attention;
}
```

### Hardware Testing

```bash
# Test on real hardware
mk install          # Install to target device
reboot              # Boot with new kernel

# Test cognitive functionality
echo "test pattern" > /cognitive/input
cat /cognitive/output

# Monitor attention allocation
cat /attention/current
echo "redistribute" > /attention/control
```

### Performance Benchmarking

```c
// benchmark.c
typedef struct Benchmark Benchmark;
struct Benchmark {
    char    *name;
    ulong   iterations;
    uvlong  (*benchfunc)(ulong);
};

uvlong
benchmark_pattern_matching(ulong iterations)
{
    uvlong start, end;
    CogPattern *pattern = createpattern();
    uchar data[] = "test pattern data";
    
    start = nsec();
    for(ulong i = 0; i < iterations; i++) {
        recognizepattern(pattern, data, sizeof(data));
    }
    end = nsec();
    
    return (end - start) / iterations; // Average time per iteration
}
```

## Debugging Cognitive Systems

### Cognitive Debugger Integration

```c
// cogdebug.c
void
cogdebugger(void)
{
    char *cmd;
    CogPattern *patterns;
    
    print("K9 Cognitive Debugger\n");
    print("Commands: patterns, attention, hypergraph, quit\n");
    
    while((cmd = readline()) != nil) {
        if(strcmp(cmd, "patterns") == 0) {
            dumppatterns();
        } else if(strcmp(cmd, "attention") == 0) {
            dumpattention();
        } else if(strcmp(cmd, "hypergraph") == 0) {
            dumphypergraph();
        } else if(strcmp(cmd, "quit") == 0) {
            break;
        } else {
            print("Unknown command: %s\n", cmd);
        }
    }
}
```

### Pattern Visualization

```c
// Visualize cognitive patterns for debugging
void
visualizepattern(CogPattern *pattern)
{
    print("Pattern ID: %lud\n", pattern->id);
    print("Attention: %lud\n", pattern->attention);
    print("Size: %lud bytes\n", pattern->size);
    
    // ASCII visualization of pattern data
    for(ulong i = 0; i < pattern->size; i++) {
        if(i % 16 == 0) print("\n%04lux: ", i);
        print("%02x ", pattern->pattern[i]);
    }
    print("\n");
}
```

## Advanced Configuration

### Kernel Configuration Options

```c
// config.h
enum {
    MAXCOGPATTERNS = 10000,     // Maximum cognitive patterns
    ATTENTIONQUANTUM = 100,     // Attention allocation quantum  
    PATTERNTHRESHOLD = 75,      // Pattern recognition threshold
    HYPERGRAPHNODES = 50000,    // Maximum hypergraph nodes
    SYMBOLTABLESIZE = 1000,     // Symbol table size
};

// Cognitive system parameters
struct CogConfig {
    ulong   maxpatterns;
    ulong   attentionquantum;
    ulong   threshold;
    ulong   hypergraphnodes;
    int     neural_symbolic;    // Enable neural-symbolic integration
    int     emergent_detection; // Enable emergent pattern detection
    int     distributed_cog;    // Enable distributed cognition
};
```

### Performance Tuning

```c
// Tune cognitive performance parameters
void
tunecognitive(void)
{
    // Adjust pattern cache size based on available memory
    ulong memsize = conf.npage * BY2PG;
    cogconfig.patterncachesize = memsize / 100; // 1% of memory
    
    // Set attention quantum based on CPU speed
    cogconfig.attentionquantum = m->cpuhz / 1000000; // 1ms quantum
    
    // Configure hypergraph parameters
    cogconfig.hypergraphnodes = min(50000, memsize / (4 * 1024));
}
```

## Contributing Guidelines

### Code Style

1. **Follow Plan 9 conventions**:
   - Use tabs for indentation
   - Avoid unnecessary complexity
   - Prefer clarity over cleverness

2. **Cognitive-specific guidelines**:
   - Document cognitive algorithms thoroughly
   - Use meaningful names for cognitive concepts
   - Provide performance characteristics

3. **Function naming**:
```c
// Good cognitive function names
recognizepattern()
allocateattention()
updatehypergraph()
emergentdetect()

// Avoid generic names
process()
handle()
manage()
```

### Submission Process

1. **Create feature branch**:
```bash
git checkout -b cognitive-feature-name
```

2. **Implement changes**:
   - Add cognitive functionality
   - Include comprehensive tests
   - Update documentation

3. **Test thoroughly**:
```bash
mk clean && mk    # Build test
runtests          # Unit tests
benchmark         # Performance tests
```

4. **Submit pull request**:
   - Describe cognitive enhancement
   - Include performance impact
   - Reference related research

### Research Integration

K9 welcomes integration of cutting-edge cognitive computing research:

- **Academic collaborations**: University research partnerships
- **Industry integration**: Commercial cognitive hardware support  
- **Open source contributions**: Community-driven cognitive algorithms
- **Standards development**: Cognitive computing standards participation

---

This guide provides the foundation for building, extending, and contributing to K9's cognitive operating system capabilities.