CC = gcc
CFLAGS = -I../src -Wall -Wextra -g

UNIT_TESTS = tests/unit/test_module1 tests/unit/test_module2

all: $(UNIT_TESTS)

tests/unit/test_%: tests/unit/test_%.c ../src/%.c
	$(CC) $(CFLAGS) -o $@ $^

run-tests: all
	@echo "Running Unit Tests..."
	@for test in $(UNIT_TESTS); do ./$$test; done

clean:
	rm -f $(UNIT_TESTS)