# Contributing to State Separator

## Project Overview

The application consists of:
1. **C++ Backend**: Core numerical algorithms
2. **PHP/HTML Frontend**: Web interface for user interaction

---

## Project Structure

```
stateseparator/
├── src/                      # C++ source code
│   ├── main.cpp              # Entry point, handles both server and test modes
│   ├── makefile              # Build configuration
│   │
│   │ # Core Algorithm
│   ├── NSeparator.cpp/h      # Main separation algorithm (singleton)
│   ├── NMixedState.cpp/h     # Quantum mixed state representation
│   ├── NPureState.cpp/h      # Quantum pure state representation
│   ├── NPeresTester.cpp/h    # Peres-Horodecki separability test
│   ├── NRandomGenerator.cpp/h# Random quantum state generation
│   │
│   │ # I/O & Utilities
│   ├── NInputHandler.cpp/h   # Matrix and parameter parsing
│   ├── NOutputHandler.cpp/h  # Result formatting
│   ├── NMathUtils.cpp/h      # Mathematical helper functions
│   ├── NError.cpp/h          # Error handling
│   ├── logger.cpp/h          # Logging utilities
│   ├── NCollector.cpp/h      # Statistics collection
│   │
│   │ # Threading
│   ├── NThreadPool.cpp/h     # Thread pool implementation
│   ├── NThreadPool_l.cpp/h   # Linux-specific threading
│   ├── NThreadPool_w.cpp     # Windows-specific threading
│   │
│   │ # Testing
│   ├── NTester.cpp/h         # Test harness for development
│   ├── NRandomizer.cpp       # Standalone random state generator
│   │
│   └── basic_def.h           # Common definitions and includes
│
├── public_html/              # Web frontend
│   ├── index.html            # Main landing page
│   ├── calculate.php         # Main PHP script (calls C++ executables)
│   ├── style.css             # Stylesheet
│   ├── documentation.html    # User documentation
│   ├── contact.html          # Contact information
│   │
│   │ # Example states
│   ├── simpleSeparable.html
│   ├── complexSeparable.html
│   ├── barelySeparable.html
│   ├── slightlyEntangled.html
│   ├── bellState.html
│   ├── wState.html
│   └── BoundEntanglementState.html
│
├── external/
│   └── Eigen/                # Eigen linear algebra library (bundled)
│
├── README.md                 # User documentation
├── CONTRIBUTING.md           # This file
├── Dockerfile                # Docker container definition
└── LICENSE                   # GPL-3.0 License
```

---

## Architecture

### Data Flow

```
┌─────────────────┐     POST request     ┌─────────────────┐
│   Browser       │ ──────────────────►  │  calculate.php  │
│   (User Input)  │                      │                 │
└─────────────────┘                      └────────┬────────┘
                                                  │
                                         exec() call with args
                                                  │
                                                  ▼
                                         ┌─────────────────┐
                                         │   NSeparator    │
                                         │   (C++ binary)  │
                                         └────────┬────────┘
                                                  │
                                          stdout output
                                                  │
                                                  ▼
┌─────────────────┐     HTML response    ┌─────────────────┐
│   Browser       │ ◄──────────────────  │  calculate.php  │
│   (Results)     │                      │                 │
└─────────────────┘                      └─────────────────┘
```

### Build Modes

The makefile supports two build modes controlled by `SERVER` variable:

| Mode | Description | Compile Flags |
|------|-------------|---------------|
| `SERVER=1` (default) | Production build for web server | `-D__NRUNONSERVER__ -D__NFULLMT__ -DNDEBUG` |
| `SERVER=0` | Development build with tests | `-D__NCOLLECTSTATS__ -D__NSTRICT__ -D__NNORANDOM__ -D__NFULLMT__` |

### Executables

1. **NSeparator**: Main executable that performs quantum state separation
   - Arguments: `particleSizes matrix targetDistance minProbForState targetNumberOfStates precision [accuracyBoost]`

2. **NRandomizer**: Generates random separable quantum states
   - Arguments: `particleSizes precision`

---

## Prerequisites

### For Native Build

- **GCC/G++** with C++11 support
- **OpenMP** for multi-threading (`libgomp`)
- **Make**

On Ubuntu/Debian:
```bash
sudo apt-get update
sudo apt-get install build-essential
```

On macOS:
```bash
brew install gcc
# Note: You may need to update the makefile to use the Homebrew gcc
```

### For Running the Web Interface

- **Apache** or **Nginx** web server
- **PHP** (5.x or 7.x) with `exec()` enabled
- The compiled binaries (`NSeparator`, `NRandomizer`) in `public_html/`

---

## Building

### Compile the C++ Backend

```bash
cd src
make clean
make all
```

This creates:
- `../build/bin/NSeparator`
- `../build/bin/NRandomizer`

### Install to Web Directory

```bash
cd src
make install
```

This copies the binaries to `public_html/`.

### Development Build

For a development build with debugging and tests:

```bash
cd src
make SERVER=0 all
```

---

## Running

### Option 1: Using Docker (Recommended)

#### Local Build (current architecture only)

```bash
# Build the Docker image
docker build -t stateseparator .

# Run the container in detached mode
docker run -d -p 8080:80 --name stateseparator stateseparator

# Access at http://localhost:8080

# To stop and remove the container:
docker stop stateseparator && docker rm stateseparator
```

#### Multi-Architecture Build (for deployment)

To build an image that works on both ARM64 (Apple Silicon) and AMD64 (Intel/AMD Linux servers):

```bash
# Create a new builder instance (one-time setup)
docker buildx create --name multiarch --use

# Build and push multi-arch image to Docker Hub (or other registry)
docker buildx build --platform linux/amd64,linux/arm64 \
  -t yourusername/stateseparator:latest \
  --push .

# Or build for a specific platform and load locally
docker buildx build --platform linux/amd64 -t stateseparator:amd64 --load .
```

To run the AMD64 image on an ARM Mac (for testing):
```bash
docker run -d -p 8080:80 --platform linux/amd64 --name stateseparator stateseparator:amd64
```

### Option 2: Local PHP Development Server

After building and installing:

```bash
cd public_html
php -S localhost:8080
```

> **Note**: PHP's built-in server is for development only.

### Option 3: Apache/Nginx

1. Build and install the binaries
2. Configure your web server to serve `public_html/`
3. Ensure PHP has permission to execute the binaries
4. Access via your configured URL

---

## Testing

### Command Line Test

After building with `SERVER=0`:

```bash
cd build/bin
./NSeparator
# Select a test scenario from the menu
```

### Manual Binary Test

```bash
cd public_html
./NSeparator "2 2" "0.25 0 0 0 0 0.25 0 0 0 0 0.25 0 0 0 0 0.25" "" "" "" "3" "0"
```

---

## Key Components Explained

### NSeparator (Main Algorithm)

The core algorithm iteratively searches for a separable matrix approximation:
1. Starts from the maximally mixed state
2. Iteratively adds pure product states
3. Uses quadratic programming (Cholesky decomposition via Eigen)
4. Returns the nearest separable state and distance

### NPeresTester

Implements the Peres-Horodecki partial-transpose test:
- Necessary condition for separability
- Sufficient for 2×2 and 2×3 systems
- Checks if partial transpose has negative eigenvalues

### NMixedState / NPureState

Quantum state representations:
- `NMixedState`: Density matrix (mixed state) with decomposition into pure states
- `NPureState`: Tensor product of particle states

---

## Testing

### Docker Integration Tests (Recommended)

Run the full integration test suite against a Docker container:

```bash
# Build the image first
docker build -t stateseparator .

# Run integration tests
./tests/test_docker.sh stateseparator
```

This tests:
- Container startup and health
- Binary execution inside container
- HTTP server response
- PHP processing
- Form submission (separable and entangled states)

### Binary Unit Tests

Run standalone binary tests inside Docker:

```bash
docker run --rm -v $(pwd)/tests:/tests stateseparator \
  /tests/test_separator.sh /var/www/html/NSeparator /var/www/html/NRandomizer
```

This tests:
- Maximally mixed state (separable)
- Bell state (entangled)
- Pure product state (separable)
- Multiple precision levels (3, 6, 9)
- 3-qubit systems
- Error handling (invalid inputs)
- Random state generation

> **Note**: Native macOS builds may fail due to deprecated POSIX semaphores (`sem_init`).
> Use Docker for reliable testing.

---

## Troubleshooting

### Build Errors

**"cannot find -lgomp"**: Install OpenMP development library
```bash
sudo apt-get install libgomp1
```

**Compiler warnings as errors**: The makefile uses `-Werror`. For development, you can temporarily remove this flag.

### Runtime Errors

**"Permission denied"**: Ensure binaries are executable
```bash
chmod +x public_html/NSeparator public_html/NRandomizer
```

**PHP exec() disabled**: Check PHP configuration
```bash
php -i | grep disable_functions
```

---

## Contributing

1. Fork the repository
2. Create a feature branch
3. Make your changes
4. Run tests: `./tests/test_docker.sh stateseparator`
5. Submit a pull request

---

## License

GPL-3.0 - See [LICENSE](LICENSE) for details.
