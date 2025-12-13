# Makefile - 基础版本
# 适用于你的项目结构:
#   include/             - 头文件目录
#   src/                - 源文件目录
#   build/              - 构建目录（自动创建）

# 编译器设置
CXX = g++

# 目录设置
SRC_DIR = src
INCLUDE_DIR = include
BUILD_DIR = build

# 使用 mysql_config 获取 MySQL 相关参数
MYSQL_CONFIG := $(shell which mysql_config 2>/dev/null)

ifneq ($(MYSQL_CONFIG),)
    MYSQL_CFLAGS := $(shell $(MYSQL_CONFIG) --cflags)
    MYSQL_LIBS := $(shell $(MYSQL_CONFIG) --libs)
else
    # 如果 mysql_config 不存在，使用默认值
    MYSQL_CFLAGS = -I/usr/include/mysql
    MYSQL_LIBS = -L/usr/lib/mysql -lmysqlclient
endif



CXXFLAGS = -Wall -Wextra -std=c++11 -g -I./include $(MYSQL_CFLAGS) \
	-DCONFIG_PATH='"/root/collection_pool/src/mysql.conf"'  #保证每个cpp编译时都能看到config_path的宏定义
LDFLAGS = $(MYSQL_LIBS)
TARGET = SQL_test



# 源文件和目标文件
SRCS = $(wildcard $(SRC_DIR)/*.cpp)
OBJS = $(SRCS:$(SRC_DIR)/%.cpp=$(BUILD_DIR)/%.o)

# 默认目标
all: $(BUILD_DIR) $(TARGET) 

# 创建构建目录
$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

# 链接生成可执行文件
$(TARGET): $(OBJS)
	$(CXX) -o $@ $^ $(LDFLAGS)
	@echo "✅ 编译完成: $(TARGET)"

# 编译每个源文件
$(BUILD_DIR)/%.o: $(SRC_DIR)/%.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# 清理
clean:
	rm -rf $(BUILD_DIR) $(TARGET)
	@echo "🧹 清理完成"

# 重新编译
rebuild: clean all

# 运行程序
run: $(TARGET)
	@echo "🚀 运行程序..."
	./$(TARGET)

# 显示帮助信息
help:
	@echo "可用命令:"
	@echo "  make all     - 编译程序（默认）"
	@echo "  make clean   - 清理构建文件"
	@echo "  make rebuild - 重新编译"
	@echo "  make run     - 运行程序"
	@echo "  make help    - 显示此帮助信息"

# 伪目标声明
.PHONY: all clean rebuild run help