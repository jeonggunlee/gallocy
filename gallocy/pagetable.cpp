#include <cstdio>
#include <cstring>
#include <string>
#include <sstream>
#include <unistd.h>

#include "heaplayers/heaptypes.h"
/*
 * TODO: we only need this import to get the types of the gallocy
 * flavored STL containers. Once that is refactored, we can include that
 * instead of the entire libgallocy.h.
 */
#include "libgallocy.h"
#include "pagetable.h"


class xSizeHeap: public SqlitexSizeHeapType {};


typedef
  std::map<void*, int,
  std::less<void*>,
  STLAllocator<std::pair<void*, int>, xSizeHeap> >
    xSizeMapType;


/**
 * This is a "page table" helper allocator that also is needed to coordinate
 * global application memory. We keep it seperate from the sqlite allocator
 * because of weird things the STL library does with memory.
 */
xSizeMapType xSizeMap;


class SqliteHeap: public SqliteAllocatorHeapType {};

/**
 * This is the "page table" used to coodinate global application memory between
 * nodes.
 */
SqliteHeap sqliteHeap;


void* xMalloc(int sz) {
  if (sz == 0) {
    return NULL;
  }
  void* ptr = sqliteHeap.malloc(sz);
  xSizeMap.insert(std::pair<void*, int>(ptr, sz));
  //fprintf(stderr, "%p = xMalloc(%d)\n", ptr, sz);
  return ptr;
}


void xFree(void* ptr) {
  //fprintf(stderr, "xFree(%p)\n", ptr);
  xSizeMap.erase(ptr);
  sqliteHeap.free(ptr);
  return;
}


void* xRealloc(void* ptr, int sz) {
  if (ptr == NULL) {
    return sqliteHeap.malloc(sz);
  }
  size_t min_size = sqliteHeap.getSize(ptr);
  void* buf = sqliteHeap.malloc(sz);
  if (buf != NULL) {
    memcpy(buf, ptr, min_size);
    sqliteHeap.free(ptr);
  }
  xSizeMap.insert(std::pair<void*, int>(buf, sz));
  //fprintf(stderr, "%p = xRealloc(%p, %d)\n", buf, ptr, sz);
  return buf;
}


int xSize(void* ptr) {
  int sz = xSizeMap[ptr];
  //fprintf(stderr, "%d = xSize(%p)\n", sz, ptr);
  return sz;
}


int xRoundup(int sz) {
  //fprintf(stderr, "xRoundup(%d)\n", sz);
  return sz;
}


int xInit(void* ptr) {
  fprintf(stderr, "xInit\n");
  return 0;
}


void xShutdown(void* ptr) {
  fprintf(stderr, "xShutdown\n");
  return;
}


sqlite3_mem_methods my_mem = {
  &xMalloc,
  &xFree,
  &xRealloc,
  &xSize,
  &xRoundup,
  &xInit,
  &xShutdown,
  0
};


void init_sqlite_memory() {
  if (sqlite3_config(SQLITE_CONFIG_MALLOC, &my_mem) != SQLITE_OK) {
    fprintf(stderr, "Failed to set custom sqlite memory allocator!\n");
    return;
  }
  fprintf(stderr, "Successfully set custom sqlite memory allocator!\n");
  return;
}


int PageTable::noop_callback(void *not_used, int argc, char **argv, char **az_col_name) {
   return 0;
}


int PageTable::print_callback(void *not_used, int argc, char **argv, char **az_col_name) {
  int i;
  for(i=0; i<argc; i++) {
    printf("%s = %s\n", az_col_name[i], argv[i] ? argv[i] : "NULL");
  }
  printf("\n");
  return 0;
}


int PageTable::condition_callback(void *cond_param, int argc, char **argv, char **az_col_name) {
  condition_callback_param *param = (condition_callback_param *) cond_param;
  pthread_mutex_lock(param->mutex);
  pthread_cond_signal(param->cv);
  pthread_mutex_unlock(param->mutex);
  return 0;
}


void PageTable::open_database() {
  int  rc;
  rc = sqlite3_open(database_path, &db);
  if(rc) {
     fprintf(stderr, "Can't open database: %s\n", sqlite3_errmsg(db));
     exit(0);
  } else {
     fprintf(stdout, "Opened database successfully\n");
  }
}


void PageTable::create_tables() {
  sqlite3_stmt* stmt;
  int  rc;

  const char *sql = \
        "CREATE TABLE pagetable (" \
        "id INT PRIMARY KEY     NOT NULL,    " \
        "address        INT     NOT NULL,    " \
        "size           INT     NOT NULL);   ";

  rc = sqlite3_prepare_v2(db, sql, strlen(sql) + 1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to create table(s)!\n");
    return;
  }

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    fprintf(stderr, "Not dont creating table! Is this bad?\n");
  }

  sqlite3_finalize(stmt);

  return;

}


void PageTable::insert_page_table_entry(void* ptr, int ptr_sz) {
  static long unique_id = 0;
  int rc;
  gallocy::stringstream sql;

  sqlite3_stmt* stmt;

  sql << "INSERT INTO pagetable (id, address, size)"
      << "VALUES (" << unique_id << ", " << (intptr_t) ptr << ", " << ptr_sz << ");";

  unique_id++;

  /* Execute SQL statement */
  rc = sqlite3_prepare_v2(db, sql.str().c_str(), sql.str().length(), &stmt, NULL);
  if(rc != SQLITE_OK) {
     fprintf(stderr, "Failed to prepare insert!\n");
  }

  if (sqlite3_step(stmt) != SQLITE_DONE) {
    fprintf(stderr, "Failed to insert page table record!\n");
  }

  sqlite3_finalize(stmt);

  return;
}


int PageTable::get_page_table_entry_count() {
  const char* sql = "SELECT COUNT(*) FROM pagetable;";
  sqlite3_stmt* stmt;
  int rc;
  int cnt = -1;
  rc = sqlite3_prepare_v2(db, sql, strlen(sql) + 1, &stmt, NULL);
  if (rc != SQLITE_OK) {
    fprintf(stderr, "Failed to prepare statement!\n");
    return -1;
  }

  while (sqlite3_step(stmt) == SQLITE_ROW) {
    cnt = sqlite3_column_int(stmt, 0);
  }
  sqlite3_finalize(stmt);
  return cnt;
}


PageTable pt;
//Scheduler s;
