CXX = g++
CXXFLAGS = -std=c++17 -Wall -g -pg -I../common/include
LDFLAGS = -pg

COMMON_SRC = \
    ../common/src/Graph.cpp \

RESULT_DIR = result
EXEC_LIST = list_exec
EXEC_DEQUE = deque_exec
EXEC_VECTOR = vector_exec

gprof_report: list deque vector
	@echo "===== GPROF Efficiency Report =====" > gprof_efficiency.txt

	@echo "[list]" >> gprof_efficiency.txt
	@rm -f $(RESULT_DIR)/gprof_list_times.txt
	@for i in 1 2 3 4 5; do \
		./$(EXEC_LIST) > /dev/null; \
		mv gmon.out $(RESULT_DIR)/gmon_list_$$i.out; \
		gprof $(EXEC_LIST) $(RESULT_DIR)/gmon_list_$$i.out > $(RESULT_DIR)/gprof_list_$$i.txt; \
		grep -E '^[[:space:]]*[0-9]+\.[0-9]+[[:space:]]+[0-9]+\.[0-9]+[[:space:]]+[0-9]+\.[0-9]+' \
		$(RESULT_DIR)/gprof_list_$$i.txt | tail -n 1 | awk '{print $$2}' >> $(RESULT_DIR)/gprof_list_times.txt; \
	done
	@awk '{ total += $$1 } END { print "Average:", total/NR, "seconds" }' $(RESULT_DIR)/gprof_list_times.txt >> gprof_efficiency.txt

	@echo "" >> gprof_efficiency.txt
	@echo "[deque]" >> gprof_efficiency.txt
	@rm -f $(RESULT_DIR)/gprof_deque_times.txt
	@for i in 1 2 3 4 5; do \
		./$(EXEC_DEQUE) > /dev/null; \
		mv gmon.out $(RESULT_DIR)/gmon_deque_$$i.out; \
		gprof $(EXEC_DEQUE) $(RESULT_DIR)/gmon_deque_$$i.out > $(RESULT_DIR)/gprof_deque_$$i.txt; \
		grep -E '^[[:space:]]*[0-9]+\.[0-9]+[[:space:]]+[0-9]+\.[0-9]+[[:space:]]+[0-9]+\.[0-9]+' \
		$(RESULT_DIR)/gprof_deque_$$i.txt | tail -n 1 | awk '{print $$2}' >> $(RESULT_DIR)/gprof_deque_times.txt; \
	done
	@awk '{ total += $$1 } END { print "Average:", total/NR, "seconds" }' $(RESULT_DIR)/gprof_deque_times.txt >> gprof_efficiency.txt

	@echo "" >> gprof_efficiency.txt
	@echo "[vector]" >> gprof_efficiency.txt
	@rm -f $(RESULT_DIR)/gprof_vector_times.txt
	@for i in 1 2 3 4 5; do \
		./$(EXEC_VECTOR) > /dev/null; \
		mv gmon.out $(RESULT_DIR)/gmon_vector_$$i.out; \
		gprof $(EXEC_VECTOR) $(RESULT_DIR)/gmon_vector_$$i.out > $(RESULT_DIR)/gprof_vector_$$i.txt; \
		grep -E '^[[:space:]]*[0-9]+\.[0-9]+[[:space:]]+[0-9]+\.[0-9]+[[:space:]]+[0-9]+\.[0-9]+' \
		$(RESULT_DIR)/gprof_vector_$$i.txt | tail -n 1 | awk '{print $$2}' >> $(RESULT_DIR)/gprof_vector_times.txt; \
	done
	@awk '{ total += $$1 } END { print "Average:", total/NR, "seconds" }' $(RESULT_DIR)/gprof_vector_times.txt >> gprof_efficiency.txt

	@echo "\n>> Based on final cumulative time in gprof output (total run time approximation)" >> gprof_efficiency.txt


# ========== std::list ==========
list: profile_list.cpp $(COMMON_SRC)
	mkdir -p $(RESULT_DIR)
	$(CXX) $(CXXFLAGS) -o $(EXEC_LIST) profile_list.cpp $(COMMON_SRC) $(LDFLAGS)
	./$(EXEC_LIST) > $(RESULT_DIR)/list.txt
	mv gmon.out $(RESULT_DIR)/gmon_list.out
	gprof $(EXEC_LIST) $(RESULT_DIR)/gmon_list.out > $(RESULT_DIR)/gprof_list.txt

# ========== std::deque ==========
deque: profile_deque.cpp $(COMMON_SRC)
	mkdir -p $(RESULT_DIR)
	$(CXX) $(CXXFLAGS) -o $(EXEC_DEQUE) profile_deque.cpp $(COMMON_SRC) $(LDFLAGS)
	./$(EXEC_DEQUE) > $(RESULT_DIR)/deque.txt
	mv gmon.out $(RESULT_DIR)/gmon_deque.out
	gprof $(EXEC_DEQUE) $(RESULT_DIR)/gmon_deque.out > $(RESULT_DIR)/gprof_deque.txt

# ========== std::vector ==========
vector: profile_vector.cpp $(COMMON_SRC)
	mkdir -p $(RESULT_DIR)
	$(CXX) $(CXXFLAGS) -o $(EXEC_VECTOR) profile_vector.cpp $(COMMON_SRC) $(LDFLAGS)
	./$(EXEC_VECTOR) > $(RESULT_DIR)/vector.txt
	mv gmon.out $(RESULT_DIR)/gmon_vector.out
	gprof $(EXEC_VECTOR) $(RESULT_DIR)/gmon_vector.out > $(RESULT_DIR)/gprof_vector.txt



clean:
	rm -f $(EXEC_LIST) $(EXEC_DEQUE) $(EXEC_VECTOR)
	
