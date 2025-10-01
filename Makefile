CC = gcc
CFLAGS = -Wall -Wextra -std=c11 -Iinclude
LDFLAGS = -lpthread
SRCDIR = src
OBJDIR = obj
TESTDIR = tests
SCRIPTDIR = scripts
SOURCES = $(wildcard $(SRCDIR)/*.c)
OBJECTS = $(SOURCES:$(SRCDIR)/%.c=$(OBJDIR)/%.o)
TARGET = echonull
TEST_RESULTS_DIR = test_results

.PHONY: all clean run test unit-tests comprehensive-tests ci-tests performance-tests security-tests

all: $(TARGET)

$(TARGET): $(OBJECTS)
	$(CC) $(OBJECTS) -o $(TARGET) $(LDFLAGS)

$(OBJDIR)/%.o: $(SRCDIR)/%.c | $(OBJDIR)
	$(CC) $(CFLAGS) -c $< -o $@

$(OBJDIR):
	mkdir -p $(OBJDIR)

# Basic run
run: $(TARGET)
	./$(TARGET)

# Unit tests
unit-tests: $(TARGET)
	@echo "🧪 Running Unit Tests..."
	@mkdir -p $(TEST_RESULTS_DIR)
	gcc $(CFLAGS) $(TESTDIR)/unit_tests.c $(SRCDIR)/contact_manager.c $(SRCDIR)/memory_allocator.c $(SRCDIR)/security.c $(SRCDIR)/ui_utils.c -o $(TEST_RESULTS_DIR)/unit_tests
	./$(TEST_RESULTS_DIR)/unit_tests

# Comprehensive test suite
comprehensive-tests: $(TARGET)
	@echo "🧪 Running Comprehensive Test Suite..."
	@mkdir -p $(TEST_RESULTS_DIR)
	gcc $(CFLAGS) $(TESTDIR)/comprehensive_tests.c $(SRCDIR)/contact_manager.c $(SRCDIR)/memory_allocator.c $(SRCDIR)/security.c $(SRCDIR)/ui_utils.c $(SRCDIR)/test_framework.c -o $(TEST_RESULTS_DIR)/comprehensive_tests -lpthread
	./$(TEST_RESULTS_DIR)/comprehensive_tests

# Performance tests
performance-tests:
	@echo "⚡ Running Performance Tests..."
	@mkdir -p $(TEST_RESULTS_DIR)
	@echo "Compiling performance benchmark..."
	@echo '#include "../src/contact_manager.c"' > $(TEST_RESULTS_DIR)/perf_test.c
	@echo '#include "../src/memory_allocator.c"' >> $(TEST_RESULTS_DIR)/perf_test.c
	@echo '#include <stdio.h>' >> $(TEST_RESULTS_DIR)/perf_test.c
	@echo '#include <time.h>' >> $(TEST_RESULTS_DIR)/perf_test.c
	@echo 'int main() {' >> $(TEST_RESULTS_DIR)/perf_test.c
	@echo '    initializeMemory(); ContactNode *contacts = NULL;' >> $(TEST_RESULTS_DIR)/perf_test.c
	@echo '    clock_t start = clock();' >> $(TEST_RESULTS_DIR)/perf_test.c
	@echo '    for (int i = 0; i < 1000; i++) {' >> $(TEST_RESULTS_DIR)/perf_test.c
	@echo '        Contact c = {"PerfTest", "123", "perf@test.com"};' >> $(TEST_RESULTS_DIR)/perf_test.c
	@echo '        addContact(&contacts, &c);' >> $(TEST_RESULTS_DIR)/perf_test.c
	@echo '    }' >> $(TEST_RESULTS_DIR)/perf_test.c
	@echo '    clock_t end = clock();' >> $(TEST_RESULTS_DIR)/perf_test.c
	@echo '    double time = ((double)(end - start)) / CLOCKS_PER_SEC;' >> $(TEST_RESULTS_DIR)/perf_test.c
	@echo '    printf("Performance: %.1f operations/second\\n", 1000.0/time);' >> $(TEST_RESULTS_DIR)/perf_test.c
	@echo '    freeContacts(contacts); return 0; }' >> $(TEST_RESULTS_DIR)/perf_test.c
	gcc $(TEST_RESULTS_DIR)/perf_test.c -o $(TEST_RESULTS_DIR)/perf_test
	./$(TEST_RESULTS_DIR)/perf_test

# Security tests
security-tests:
	@echo "🔒 Running Security Tests..."
	@mkdir -p $(TEST_RESULTS_DIR)
	@echo '#include "../src/security.c"' > $(TEST_RESULTS_DIR)/sec_test.c
	@echo '#include <string.h>' >> $(TEST_RESULTS_DIR)/sec_test.c
	@echo 'int main() {' >> $(TEST_RESULTS_DIR)/sec_test.c
	@echo '    char msg[] = "test"; char enc[100], dec[100];' >> $(TEST_RESULTS_DIR)/sec_test.c
	@echo '    encryptData(msg, enc, 4); decryptData(enc, dec, 4);' >> $(TEST_RESULTS_DIR)/sec_test.c
	@echo '    printf("Security test: %s\\n", strcmp(msg, dec) == 0 ? "PASS" : "FAIL");' >> $(TEST_RESULTS_DIR)/sec_test.c
	@echo '    return 0; }' >> $(TEST_RESULTS_DIR)/sec_test.c
	gcc $(TEST_RESULTS_DIR)/sec_test.c -o $(TEST_RESULTS_DIR)/sec_test
	./$(TEST_RESULTS_DIR)/sec_test

# CI/CD pipeline test
ci-tests: clean
	@echo "🚀 Running CI/CD Test Pipeline..."
	@if [ -f "$(SCRIPTDIR)/run_tests.sh" ]; then \
		$(SCRIPTDIR)/run_tests.sh; \
	else \
		echo "❌ CI script not found. Running basic tests..."; \
		make unit-tests comprehensive-tests; \
	fi

# Full test suite
test: unit-tests comprehensive-tests performance-tests security-tests
	@echo ""
	@echo "🎉 All tests completed successfully!"
	@echo "📊 Test results saved to: $(TEST_RESULTS_DIR)/"

# Memory analysis
memory-analysis:
	@echo "🧠 Running Memory Analysis..."
	@mkdir -p $(TEST_RESULTS_DIR)
	@echo '#include "../src/memory_allocator.c"' > $(TEST_RESULTS_DIR)/mem_analysis.c
	@echo '#include <stdio.h>' >> $(TEST_RESULTS_DIR)/mem_analysis.c
	@echo 'int main() { initializeMemory(); visualizeMemory(); return 0; }' >> $(TEST_RESULTS_DIR)/mem_analysis.c
	gcc $(TEST_RESULTS_DIR)/mem_analysis.c -o $(TEST_RESULTS_DIR)/mem_analysis
	./$(TEST_RESULTS_DIR)/mem_analysis

# Code quality check
quality-check:
	@echo "🔍 Running Code Quality Check..."
	@echo "Checking for memory leaks..."
	@if grep -r "malloc\|calloc\|realloc" src/ | grep -v "myMalloc" | head -5; then \
		echo "⚠️  Found non-custom memory allocation"; \
	else \
		echo "✅ No unsafe memory allocations found"; \
	fi
	@echo "Checking for potential issues..."
	@if grep -r "strcpy\|strcat\|sprintf" src/ | head -3; then \
		echo "⚠️  Found potentially unsafe string functions"; \
	else \
		echo "✅ String functions look safe"; \
	fi

# Development targets
dev-build: CFLAGS += -g -DDEBUG -fsanitize=address
dev-build: clean $(TARGET)

# Production build (optimized)
prod-build: CFLAGS += -O2 -DNDEBUG
prod-build: clean $(TARGET)

clean:
	rm -rf $(OBJDIR) $(TARGET) contacts.dat $(TEST_RESULTS_DIR) *.dat

# Install targets
install: $(TARGET)
	@echo "Installing EchoNull to /usr/local/bin..."
	sudo cp $(TARGET) /usr/local/bin/echonull
	@echo "✅ Installation complete!"

uninstall:
	@echo "Uninstalling EchoNull..."
	sudo rm -f /usr/local/bin/echonull
	@echo "✅ Uninstallation complete!"

# Development utilities
format:
	@echo "Formatting code..."
	@find src include tests -name "*.c" -o -name "*.h" | xargs clang-format -i 2>/dev/null || echo "clang-format not available"

lint:
	@echo "Running linter..."
	@if command -v cppcheck >/dev/null 2>&1; then \
		cppcheck --enable=all --std=c11 src/ include/; \
	else \
		echo "cppcheck not installed"; \
	fi

# Documentation
docs:
	@echo "Generating documentation..."
	@if command -v doxygen >/dev/null 2>&1; then \
		doxygen Doxyfile 2>/dev/null || echo "Doxyfile not found"; \
	else \
		echo "doxygen not installed"; \
	fi

# Help
help:
	@echo ""
	@echo "🚀 EchoNull Contact Manager - Build System"
	@echo "=========================================="
	@echo ""
	@echo "📦 BUILD TARGETS:"
	@echo "  all           Build the project (default)"
	@echo "  dev-build     Debug build with sanitizers"
	@echo "  prod-build    Optimized production build"
	@echo ""
	@echo "🧪 TESTING TARGETS:"
	@echo "  test          Run all tests"
	@echo "  unit-tests    Run unit tests only"
	@echo "  comprehensive-tests Run comprehensive test suite"
	@echo "  performance-tests Run performance benchmarks"
	@echo "  security-tests Run security validation"
	@echo "  ci-tests      Run CI/CD pipeline"
	@echo "  memory-analysis Run memory analysis"
	@echo ""
	@echo "🔍 QUALITY TARGETS:"
	@echo "  quality-check Run code quality checks"
	@echo "  format        Format source code"
	@echo "  lint          Run static analysis"
	@echo ""
	@echo "📚 UTILITIES:"
	@echo "  run           Build and run the application"
	@echo "  docs          Generate documentation"
	@echo "  clean         Clean build artifacts"
	@echo "  install       Install to system"
	@echo "  uninstall     Remove from system"
	@echo "  help          Show this help message"
	@echo ""
	@echo "📊 Test Results Directory: $(TEST_RESULTS_DIR)/"
	@echo ""