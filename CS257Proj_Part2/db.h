/********************************************************************
db.h - This file contains all the structures, defines, and function
	prototype for the db.exe program.
*********************************************************************/

#define MAX_IDENT_LEN   16
#define MAX_NUM_COL			16
#define MAX_TOK_LEN			32
#define KEYWORD_OFFSET	10
#define STRING_BREAK		" (),<>="
#define NUMBER_BREAK		" ),"

/* Column descriptor sturcture = 20+4+4+4+4 = 36 bytes */
typedef struct cd_entry_def
{
	char		col_name[MAX_IDENT_LEN+4];
	int			col_id;                   /* Start from 0 */
	int			col_type;
	int			col_len;
	int 		not_null;
} cd_entry;

/* Table packed descriptor sturcture = 4+20+4+4+4 = 36 bytes
   Minimum of 1 column in a table - therefore minimum size of
	 1 valid tpd_entry is 36+36 = 72 bytes. */
typedef struct tpd_entry_def
{
	int				tpd_size;
	char			table_name[MAX_IDENT_LEN+4];
	int				num_columns;
	int				cd_offset;
	int       tpd_flags;
} tpd_entry;

/* Table packed descriptor list = 4+4+4+36 = 48 bytes.  When no
   table is defined the tpd_list is 48 bytes.  When there is 
	 at least 1 table, then the tpd_entry (36 bytes) will be
	 overlapped by the first valid tpd_entry. */
typedef struct tpd_list_def
{
	int				list_size;
	int				num_tables;
	int				db_flags;
	tpd_entry	tpd_start;
}tpd_list;

/* This token_list definition is used for breaking the command
   string into separate tokens in function get_tokens().  For
	 each token, a new token_list will be allocated and linked 
	 together. */
typedef struct t_list
{
	char	tok_string[MAX_TOK_LEN];
	int		tok_class;
	int		tok_value;
	struct t_list *next;
} token_list;

/* This enum defines the different classes of tokens for 
	 semantic processing. */
typedef enum t_class
{
	keyword = 1,	// 1
	identifier,		// 2
	symbol, 			// 3
	type_name,		// 4
	constant,		  // 5
  function_name,// 6
	terminator,		// 7
	error			    // 8
  
} token_class;

/* This enum defines the different values associated with
   a single valid token.  Use for semantic processing. */
typedef enum t_value
{
	T_INT = 10,		// 10 - new type should be added above this line
	T_VARCHAR,		    // 11 
	T_CHAR,		    // 12       
	K_CREATE, 		// 13
	K_TABLE,			// 14
	K_NOT,				// 15
	K_NULL,				// 16
	K_DROP,				// 17
	K_LIST,				// 18
	K_SCHEMA,			// 19
  K_FOR,        // 20
	K_TO,				  // 21
  K_INSERT,     // 22
  K_INTO,       // 23
  K_VALUES,     // 24
  K_DELETE,     // 25
  K_FROM,       // 26
  K_WHERE,      // 27
  K_UPDATE,     // 28
  K_SET,        // 29
  K_SELECT,     // 30
  K_ORDER,      // 31
  K_BY,         // 32
  K_DESC,       // 33
  K_IS,         // 34
  K_AND,        // 35
  K_OR,         // 36
  K_NATURAL,    // 37
  K_JOIN,       // 38 - new keyword should be added below this line
  F_SUM,        // 39
  F_AVG,        // 40
	F_COUNT,      // 41 - new function name should be added below this line
	S_LEFT_PAREN = 70,  // 70
	S_RIGHT_PAREN,		  // 71
	S_COMMA,			      // 72
  S_STAR,             // 73
  S_EQUAL,            // 74
  S_LESS,             // 75
  S_GREATER,          // 76
	IDENT = 85,			    // 85
	INT_LITERAL = 90,	  // 90
  STRING_LITERAL,     // 91
	EOC = 95,			      // 95
	INVALID = 99		    // 99
} token_value;

/* This constants must be updated when add new keywords */
#define TOTAL_KEYWORDS_PLUS_TYPE_NAMES 32

/* New keyword must be added in the same position/order as the enum
   definition above, otherwise the lookup will be wrong */
const char *keyword_table[] = 
{
  "int", "varchar", "char", "create", "table", "not", "null", "drop", "list", "schema",
  "for", "to", "insert", "into", "values", "delete", "from", "where", 
  "update", "set", "select", "order", "by", "desc", "is", "and", "or",
  "natural", "join", "sum", "avg", "count"
};

/* This enum defines a set of possible statements */
typedef enum s_statement
{
  INVALID_STATEMENT = -199,	// -199
	CREATE_TABLE = 100,				// 100
	DROP_TABLE,								// 101
	LIST_TABLE,								// 102
	LIST_SCHEMA,							// 103
  INSERT,                   // 104
  DELETE,                   // 105
  UPDATE,                   // 106
  SELECT                    // 107
} semantic_statement;

/* This enum has a list of all the errors that should be detected
   by the program.  Can append to this if necessary. */
typedef enum error_return_codes
{
	INVALID_TABLE_NAME = -399,	// -399
	DUPLICATE_TABLE_NAME,				// -398
	TABLE_NOT_EXIST,						// -397
	INVALID_TABLE_DEFINITION,		// -396
	INVALID_COLUMN_NAME,				// -395
	DUPLICATE_COLUMN_NAME,			// -394
	COLUMN_NOT_EXIST,						// -393
	MAX_COLUMN_EXCEEDED,				// -392
	INVALID_TYPE_NAME,					// -391
	INVALID_COLUMN_DEFINITION,	// -390
	INVALID_COLUMN_LENGTH,			// -389
  INVALID_REPORT_FILE_NAME,		// -388
  INVALID_AGGREGATE_COLUMN,     // -387
  TOO_MANY_COLUMNS,             // -386
  /* Must add all the possible errors from I/U/D + SELECT here */
	FILE_OPEN_ERROR = -299,			// -299
	DBFILE_CORRUPTION,					// -298
	MEMORY_ERROR							  // -297
} return_codes;

/* WHERE clause operator types */
typedef enum where_operator_type
{
	OP_EQUAL = 1,       // =
	OP_LESS,            // <
	OP_GREATER,         // >
	OP_IS_NULL,         // IS NULL
	OP_IS_NOT_NULL      // IS NOT NULL
} where_operator;

/* Logical operators for chaining conditions */
typedef enum logical_operator_type
{
	LOG_NONE = 0,       // No logical operator (single condition)
	LOG_AND,            // AND
	LOG_OR              // OR
} logical_operator;

/* Single WHERE condition structure */
typedef struct where_condition_def
{
	char col_name[MAX_IDENT_LEN+1];     // Column name
	where_operator op;                   // Operator type
	char value[MAX_TOK_LEN];             // Comparison value (for relational ops)
	int col_index;                       // Column index in table (resolved at parse time)
	logical_operator log_op;             // Logical operator to next condition
	struct where_condition_def *next;    // Next condition in chain
} where_condition;

/* SELECT column specification */
typedef enum select_col_type_enum
{
	SEL_STAR = 1,           // SELECT *
	SEL_COLUMNS,            // SELECT col1, col2, ...
	SEL_AGGREGATE           // SELECT AGG(col)
} select_col_type;

/* Aggregate function types */
typedef enum aggregate_type_enum
{
	AGG_NONE = 0,
	AGG_SUM,
	AGG_AVG,
	AGG_COUNT
} aggregate_type;

/* SELECT query structure */
typedef struct select_query_def
{
	select_col_type col_type;
	int col_indices[MAX_NUM_COL];       // Column indices to select
	int num_columns;                     // Number of columns selected
	aggregate_type agg_type;            // Aggregate function type
	int agg_col_index;                  // Column index for aggregate
	char agg_col_name[MAX_IDENT_LEN+1]; // Column name for aggregate
	char order_by_col[MAX_IDENT_LEN+1]; // ORDER BY column name
	int order_by_col_index;             // ORDER BY column index
	bool order_desc;                    // Descending order flag
} select_query;


/* This structure defines the table file header that will be
   stored at the beginning of each table data file. */	
typedef struct table_file_header_def
{
  int         file_size;        // total bytes in file
  int         record_size;      // fixed size of each record (rounded to 4B)
  int         num_records;      // count of records currently stored
  int         record_offset;    // byte offset where record area begins
  int         file_header_flag; // reserved for future use
  tpd_entry  *tpd_ptr;          // transient pointer; must be 0 before writing
} table_file_header;



/* Set of function prototypes */
int get_token(char *command, token_list **tok_list);
void add_to_list(token_list **tok_list, const char *tmp, int t_class, int t_value);
int do_semantic(token_list *tok_list);
int sem_create_table(token_list *t_list);
int sem_drop_table(token_list *t_list);
int sem_list_tables();
int sem_list_schema(token_list *t_list);
int sem_insert(token_list *t_list);
int sem_select_stmt(token_list *t_list);

/*
	Keep a global list of tpd - in real life, this will be stored
	in shared memory.  Build a set of functions/methods around this.
*/
tpd_list	*g_tpd_list;
int initialize_tpd_list();
int add_tpd_to_list(tpd_entry *tpd);
int drop_tpd_from_list(char *tabname);
tpd_entry* get_tpd_from_list(char *tabname);
int reload_tpd_list();


// Functions for table file handling
// db.h  (replace your whole “Functions for table file handling” block with this)
int calc_record_size(const tpd_entry *tpd);          // returns 4-byte aligned size
int create_table_file(const tpd_entry *tpd);         // writes <table>.tab header
int drop_table_file(const char *table_name);         // removes <table>.tab

// Functions for table file header handling
int load_table_header(const char *table_name, table_file_header *out_hdr);
int save_table_header(const char *table_name, const table_file_header *hdr);
long data_area_pos(const table_file_header *hdr, int row_index); // byte offset


// Functions for packing/unpacking rows
int pack_row_from_values(const tpd_entry *tpd,
                         const char **values,  // array of strings, "NULL" allowed, ints as decimal text
                         unsigned char *out_record, int out_len);

int unpack_row_to_strings(const tpd_entry *tpd,
                          const unsigned char *record,
                          char **out_strings,  // caller allocates per-column buffers
                          int out_cap_each);   // capacity per buffer

// Function to append a record to a table file
int append_record(const char *table_name, const unsigned char *record, int rec_len);

// High-level functions for DML operations
int insert_values(const char *table_name, const char **values, int value_count);
int select_all(const char *table_name);
int select_natural_join(const char *table1_name, const char *table2_name);

int sem_insert(token_list *t_list);
int sem_select_stmt(token_list *t_list);
int sem_delete(token_list *t_list);
int sem_update(token_list *t_list);

// WHERE clause parsing and evaluation
where_condition* parse_where_clause(token_list **cur_token, tpd_entry *tpd);
bool evaluate_where_condition(where_condition *cond, char **row_data, tpd_entry *tpd);
void free_where_conditions(where_condition *cond);

// DELETE and UPDATE functions
int delete_records(const char *table_name, where_condition *where_cond);
int update_records(const char *table_name, const char *set_col, const char *set_value, where_condition *where_cond);