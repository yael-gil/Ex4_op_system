# ===== Makefile =====

# קומפיילר ודגלים
CXX       ?= g++
CXXFLAGS  ?= -std=c++17 -O2 -Wall -Wextra -Wpedantic -MMD -MP
LDFLAGS   ?=
# מצב הדיבוג: הפעלי עם "make DBG=1" כדי לקבל סמלי דיבוג וללא אופטימיזציה
ifeq ($(DBG),1)
  CXXFLAGS := -std=c++17 -O0 -g -Wall -Wextra -Wpedantic -MMD -MP -DDEBUG
endif

# קבצי מקור
COMMON_SRCS   := Graph.cpp
SERVER_SRCS   := server.cpp
CLIENT_SRCS   := client.cpp

# אובייקטים ותלויות
COMMON_OBJS   := $(COMMON_SRCS:.cpp=.o)
SERVER_OBJS   := $(SERVER_SRCS:.cpp=.o) $(COMMON_OBJS)
CLIENT_OBJS   := $(CLIENT_SRCS:.cpp=.o) $(COMMON_OBJS)

COMMON_DEPS   := $(COMMON_OBJS:.o=.d)
SERVER_DEPS   := $(SERVER_OBJS:.o=.d)
CLIENT_DEPS   := $(CLIENT_OBJS:.o=.d)

# שמות בינאריים
SERVER_BIN    := server
CLIENT_BIN    := client

# יעד ברירת מחדל
.PHONY: all
all: $(SERVER_BIN) $(CLIENT_BIN)

# כללים ללינק
$(SERVER_BIN): $(SERVER_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

$(CLIENT_BIN): $(CLIENT_OBJS)
	$(CXX) $(CXXFLAGS) $^ -o $@ $(LDFLAGS)

# כלל קומפילציה גנרי לקבצי .cpp -> .o
%.o: %.cpp
	$(CXX) $(CXXFLAGS) -c $< -o $@

# ניקוי
.PHONY: clean
clean:
	$(RM) -f $(SERVER_BIN) $(CLIENT_BIN) \
		*.o *.d

# ניקוי מלא (כולל קובץ סוקט אם נוצר)
.PHONY: distclean
distclean: clean
	$(RM) -f mysocket

# עזרה קצרה
.PHONY: help
help:
	@echo "יעדים זמינים:"
	@echo "  make              - בונה את השרת והקליינט"
	@echo "  make clean        - מנקה אובייקטים ובינאריים"
	@echo "  make distclean    - clean + מוחק mysocket"
	@echo "  make DBG=1        - בנייה במצב דיבוג (-g, ללא אופטימיזציה)"

# כלול קבצי תלויות אוטומטיים (נוצרים ע\"י -MMD -MP)
-include $(SERVER_DEPS) $(CLIENT_DEPS) $(COMMON_DEPS)
