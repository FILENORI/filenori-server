#!/bin/bash

# 사용할 도커 이미지 이름과 컨테이너 이름을 지정합니다.
IMAGE_NAME="filenori_server"
CONTAINER_NAME="filenori_server_container"
PORT=12345

# 1) 도커 이미지 빌드
echo "[INFO] Building Docker image: $IMAGE_NAME"
docker build -t $IMAGE_NAME .

# 2) 기존에 동일 이름의 컨테이너가 있으면 종료 후 삭제
echo "[INFO] Stopping/removing old container (if any)."
docker stop $CONTAINER_NAME 2>/dev/null || true
docker rm $CONTAINER_NAME 2>/dev/null || true

# 3) 새 컨테이너 실행 (재부팅 시 자동 재시작되도록 설정)
echo "[INFO] Running new container with auto-restart policy."
docker run -d \
    --name $CONTAINER_NAME \
    --restart=unless-stopped \
    -p $PORT:$PORT \
    $IMAGE_NAME

echo "[INFO] Container is running in background."
echo "[INFO] Access the server via localhost:$PORT"