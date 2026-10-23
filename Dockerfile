# State Separator - Quantum State Separability Checker
# Multi-stage build: compile C++ backend, then serve with Apache+PHP
#
# Supports multi-architecture builds (linux/amd64, linux/arm64)
# Build with: docker buildx build --platform linux/amd64,linux/arm64 -t stateseparator .

# =============================================================================
# Stage 1: Build the C++ executables
# =============================================================================
FROM debian:bookworm-slim AS builder

# Install build dependencies
RUN apt-get update && apt-get install -y --no-install-recommends \
    build-essential \
    g++ \
    make \
    libgomp1 \
    && rm -rf /var/lib/apt/lists/*

# Copy source files
WORKDIR /app
COPY src/ /app/src/
COPY external/ /app/external/

# Build the executables
WORKDIR /app/src
RUN make clean && make all SERVER=1

# =============================================================================
# Stage 2: Runtime environment with Apache + PHP
# =============================================================================
# 8.2.35 fixes CVE-2026-109618/109619. Pinned by digest of the multi-arch index (as of 2026-10-23).
FROM php:8.2.35-apache-bookworm@sha256:5714c8607608e50d880a5f222fab556edecd4a11b9c50c81fce1c7932c1cf604

# Install runtime dependencies (OpenMP runtime library)
RUN apt-get update && apt-get install -y --no-install-recommends \
    libgomp1 \
    && rm -rf /var/lib/apt/lists/*

# PHP hardening: expose_php=Off, session.upload_progress.enabled=0
COPY docker/php.ini $PHP_INI_DIR/conf.d/zz-stateseparator.ini

# Copy the compiled binaries from builder stage
COPY --from=builder /app/build/bin/NSeparator /var/www/html/NSeparator
COPY --from=builder /app/build/bin/NRandomizer /var/www/html/NRandomizer

# Copy web files
COPY public_html/ /var/www/html/

# Make binaries executable
RUN chmod +x /var/www/html/NSeparator /var/www/html/NRandomizer

# Set proper ownership
RUN chown -R www-data:www-data /var/www/html

# Enable Apache modules if needed (mod_rewrite for clean URLs if desired)
RUN a2enmod rewrite

# Expose port 80
EXPOSE 80

# Start Apache in foreground
CMD ["apache2-foreground"]
