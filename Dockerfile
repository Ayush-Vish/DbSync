# Build stage
FROM ubuntu:24.04 AS builder

ENV DEBIAN_FRONTEND=noninteractive

# Install build dependencies
RUN apt-get update && apt-get install -y \
    build-essential \
    liburing-dev \
    pkg-config \
    cmake \
    libabsl-dev \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /build

# Copy source code
COPY src/ /build/src/
COPY include/ /build/include/

# Build the application
RUN ABSL_LIBS=$(pkg-config --libs absl_flat_hash_map absl_hash absl_city absl_raw_hash_set 2>/dev/null || echo "-labsl_hash -labsl_city -labsl_raw_hash_set -labsl_low_level_hash") && \
    g++ -std=c++20 -O3 src/main.cpp src/reactor.cpp -Iinclude -o DbSync -luring -pthread $ABSL_LIBS

# Final image
FROM ubuntu:24.04

# Install runtime dependencies
RUN apt-get update && apt-get install -y \
    liburing2 \
    && rm -rf /var/lib/apt/lists/*

WORKDIR /app

# Copy the built binary
COPY --from=builder /build/DbSync .

# Allow connections on the standard Redis port
EXPOSE 6379

CMD ["./DbSync"]
