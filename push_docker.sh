#!/bin/bash
set -e

echo "======================================"
echo " DbSync Docker Push Script "
echo "======================================"

# Check if image name is provided
if [ -z "$1" ]; then
    echo "Usage: ./push_docker.sh <dockerhub-username>"
    echo "Example: ./push_docker.sh myusername"
    exit 1
fi

DOCKER_USER=$1
IMAGE_NAME="dbsync"
TAG="latest"
FULL_IMAGE_NAME="$DOCKER_USER/$IMAGE_NAME:$TAG"

echo "1. Building the Docker image (if not already built)..."
docker build -t $IMAGE_NAME .

echo "2. Tagging the image for Docker Hub..."
docker tag $IMAGE_NAME $FULL_IMAGE_NAME

echo "3. Please log in to Docker Hub if you haven't already."
echo "Running 'docker login'..."
docker login

echo "4. Pushing image to Docker Hub ($FULL_IMAGE_NAME)..."
docker push $FULL_IMAGE_NAME

echo "======================================"
echo "✅ Successfully pushed $FULL_IMAGE_NAME to Docker Hub!"
echo "   You can now run it anywhere with:"
echo "   docker run -p 6379:6379 -d $FULL_IMAGE_NAME"
echo "======================================"
