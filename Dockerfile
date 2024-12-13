# Step 1: Select a base image
FROM ubuntu:22.04

# Step 2: Install build dependencies
RUN apt-get update && apt-get install -y \
    g++ \
    cmake \
    make \
    libboost-all-dev \
    curl \
    nlohmann-json3-dev \
    && apt-get clean

# Step 3: Set the working directory
WORKDIR /app

# Step 4: Copy only required files
COPY . /app

# Step 5: Build the application
RUN cmake . && make

# Step 6: Expose the port
EXPOSE 12345

# Step 7: Command to run the application
CMD ["./boost_asio_example"]
