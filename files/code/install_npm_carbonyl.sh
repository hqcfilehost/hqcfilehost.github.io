#!/bin/bash
# NOI Linux (Ubuntu) 环境下安装 npm 和 carbonyl 的脚本
# 用法: bash install_npm_carbonyl.sh

set -euo pipefail

echo "=== 更新 apt 包索引 ==="
apt update

echo "=== 安装基础依赖 ==="
apt install -y \
    curl \
    wget \
    gnupg \
    ca-certificates \
    build-essential \
    python3 \
    python3-pip \
    pkg-config \
    libssl-dev \
    libsqlite3-dev \
    libreadline-dev \
    libffi-dev \
    libbz2-dev \
    liblzma-dev \
    zlib1g-dev

echo "=== 安装 npm (Node.js 包管理器) ==="
apt install -y npm

echo "=== 验证 npm 安装 ==="
npm --version

echo "=== 安装 carbonyl ==="
npm install -g carbonyl

echo "=== 验证 carbonyl 安装 ==="
carbonyl --version

echo "=== 安装完成 ==="
echo "现在可以使用 carbonyl 命令渲染网页了"
echo "示例: carbonyl https://example.com"