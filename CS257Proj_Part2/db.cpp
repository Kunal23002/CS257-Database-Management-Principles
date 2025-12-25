/************************************************************
	Project#1:	CLP & DDL Kunal 
 ************************************************************/

#include "db.h"
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>
#include <sys/stat.h>
#include <sys/types.h>
#include <vector>
#include <string>
#include <algorithm>  // for std::sort
#include <cstdio>
#include <cstring>
#include <cstdlib>   // for std::atoi

#if defined(_WIN32) || defined(_WIN64)
#define strcasecmp _stricmp
#endif

int main(int argc, char** argv)
{
	int rc = 0;
	token_list *tok_list=NULL, *tok_ptr=NULL, *tmp_tok_ptr=NULL;

	if ((argc != 2) || (strlen(argv[1]) == 0))
	{
		printf("Usage: db \"command statement\"\n");
		return 1;
	}

	rc = initialize_tpd_list();

  if (rc)
  {
		printf("\nError in initialize_tpd_list().\nrc = %d\n", rc);
  }
	else
	{
    rc = get_token(argv[1], &tok_list);

		/* Test code */
		tok_ptr = tok_list;
		while (tok_ptr != NULL)
		{
			printf("%16s \t%d \t %d\n",tok_ptr->tok_string, tok_ptr->tok_class,
				      tok_ptr->tok_value);
			tok_ptr = tok_ptr->next;
		}
    
		if (!rc)
		{
			rc = do_semantic(tok_list);
		}

		if (rc)
		{
			tok_ptr = tok_list;
			while (tok_ptr != NULL)
			{
				if ((tok_ptr->tok_class == error) ||
					  (tok_ptr->tok_value == INVALID))
				{
					printf("\nError in the string: %s\n", tok_ptr->tok_string);
					printf("rc=%d\n", rc);
					break;
				}
				tok_ptr = tok_ptr->next;
			}
		}

    /* Whether the token list is valid or not, we need to free the memory */
		tok_ptr = tok_list;
		while (tok_ptr != NULL)
		{
      tmp_tok_ptr = tok_ptr->next;
      free(tok_ptr);
      tok_ptr=tmp_tok_ptr;
		}
	}

	return rc;
}

/************************************************************* 
	This is a lexical analyzer for simple SQL statements
 *************************************************************/
int get_token(char* command, token_list** tok_list)
{
	int rc=0,i,j;
	char *start, *cur, temp_string[MAX_TOK_LEN];
	bool done = false;
	
	start = cur = command;
	while (!done)
	{
		bool found_keyword = false;

		/* This is the TOP Level for each token */
	  memset ((void*)temp_string, '\0', MAX_TOK_LEN);
		i = 0;

		/* Get rid of all the leading blanks */
		while (*cur == ' ')
			cur++;

		if (cur && isalpha(*cur))
		{
			// find valid identifier
			int t_class;
			do 
			{
				temp_string[i++] = *cur++;
			}
			while ((isalnum(*cur)) || (*cur == '_'));

			if (!(strchr(STRING_BREAK, *cur)))
			{
				/* If the next char following the keyword or identifier
				   is not a blank, (, ), or a comma, then append this
					 character to temp_string, and flag this as an error */
				temp_string[i++] = *cur++;
				add_to_list(tok_list, temp_string, error, INVALID);
				rc = INVALID;
				done = true;
			}
			else
			{

				// We have an identifier with at least 1 character
				// Now check if this ident is a keyword
				for (j = 0, found_keyword = false; j < TOTAL_KEYWORDS_PLUS_TYPE_NAMES; j++)
				{
					if ((strcasecmp(keyword_table[j], temp_string) == 0))
					{
						found_keyword = true;
						break;
					}
				}

				if (found_keyword)
				{
				  if (KEYWORD_OFFSET+j < K_CREATE)
						t_class = type_name;
					else if (KEYWORD_OFFSET+j >= F_SUM)
            t_class = function_name;
          else
					  t_class = keyword;

					add_to_list(tok_list, temp_string, t_class, KEYWORD_OFFSET+j);
				}
				else
				{
					if (strlen(temp_string) <= MAX_IDENT_LEN)
					  add_to_list(tok_list, temp_string, identifier, IDENT);
					else
					{
						add_to_list(tok_list, temp_string, error, INVALID);
						rc = INVALID;
						done = true;
					}
				}

				if (!*cur)
				{
					add_to_list(tok_list, "", terminator, EOC);
					done = true;
				}
			}
		}
		else if (isdigit(*cur))
		{
			// find valid number
			do 
			{
				temp_string[i++] = *cur++;
			}
			while (isdigit(*cur));

			if (!(strchr(NUMBER_BREAK, *cur)))
			{
				/* If the next char following the keyword or identifier
				   is not a blank or a ), then append this
					 character to temp_string, and flag this as an error */
				temp_string[i++] = *cur++;
				add_to_list(tok_list, temp_string, error, INVALID);
				rc = INVALID;
				done = true;
			}
			else
			{
				add_to_list(tok_list, temp_string, constant, INT_LITERAL);

				if (!*cur)
				{
					add_to_list(tok_list, "", terminator, EOC);
					done = true;
				}
			}
		}
		else if ((*cur == '(') || (*cur == ')') || (*cur == ',') || (*cur == '*')
		         || (*cur == '=') || (*cur == '<') || (*cur == '>'))
		{
			/* Catch all the symbols here. Note: no look ahead here. */
			int t_value;
			switch (*cur)
			{
				case '(' : t_value = S_LEFT_PAREN; break;
				case ')' : t_value = S_RIGHT_PAREN; break;
				case ',' : t_value = S_COMMA; break;
				case '*' : t_value = S_STAR; break;
				case '=' : t_value = S_EQUAL; break;
				case '<' : t_value = S_LESS; break;
				case '>' : t_value = S_GREATER; break;
			}

			temp_string[i++] = *cur++;

			add_to_list(tok_list, temp_string, symbol, t_value);

			if (!*cur)
			{
				add_to_list(tok_list, "", terminator, EOC);
				done = true;
			}
		}
    else if (*cur == '\'')
    {
      /* Find STRING_LITERRAL */
			int t_class;
      cur++;
			do 
			{
				temp_string[i++] = *cur++;
			}
			while ((*cur) && (*cur != '\''));

      temp_string[i] = '\0';

			if (!*cur)
			{
				/* If we reach the end of line */
				add_to_list(tok_list, temp_string, error, INVALID);
				rc = INVALID;
				done = true;
			}
      else /* must be a ' */
      {
        add_to_list(tok_list, temp_string, constant, STRING_LITERAL);
        cur++;
				if (!*cur)
				{
					add_to_list(tok_list, "", terminator, EOC);
					done = true;
        }
      }
    }
		else
		{
			if (!*cur)
			{
				add_to_list(tok_list, "", terminator, EOC);
				done = true;
			}
			else
			{
				/* not a ident, number, or valid symbol */
				temp_string[i++] = *cur++;
				add_to_list(tok_list, temp_string, error, INVALID);
				rc = INVALID;
				done = true;
			}
		}
	}
			
  return rc;
}

void add_to_list(token_list **tok_list, const char *tmp, int t_class, int t_value)
{
	token_list *cur = *tok_list;
	token_list *ptr = NULL;

	// printf("%16s \t%d \t %d\n",tmp, t_class, t_value);

	ptr = (token_list*)calloc(1, sizeof(token_list));
	strcpy(ptr->tok_string, tmp);
	ptr->tok_class = t_class;
	ptr->tok_value = t_value;
	ptr->next = NULL;

  if (cur == NULL)
		*tok_list = ptr;
	else
	{
		while (cur->next != NULL)
			cur = cur->next;

		cur->next = ptr;
	}
	return;
}

int do_semantic(token_list *tok_list)
{
	int rc = 0, cur_cmd = INVALID_STATEMENT;
	bool unique = false;
  token_list *cur = tok_list;

	if ((cur->tok_value == K_CREATE) &&
			((cur->next != NULL) && (cur->next->tok_value == K_TABLE)))
	{
		printf("CREATE TABLE statement\n");
		cur_cmd = CREATE_TABLE;
		cur = cur->next->next;
	}
	else if ((cur->tok_value == K_DROP) &&
					((cur->next != NULL) && (cur->next->tok_value == K_TABLE)))
	{
		printf("DROP TABLE statement\n");
		cur_cmd = DROP_TABLE;
		cur = cur->next->next;
	}
	else if ((cur->tok_value == K_LIST) &&
					((cur->next != NULL) && (cur->next->tok_value == K_TABLE)))
	{
		printf("LIST TABLE statement\n");
		cur_cmd = LIST_TABLE;
		cur = cur->next->next;
	}
	else if ((cur->tok_value == K_LIST) &&
					((cur->next != NULL) && (cur->next->tok_value == K_SCHEMA)))
	{
		printf("LIST SCHEMA statement\n");
		cur_cmd = LIST_SCHEMA;
		cur = cur->next->next;
	}
	else if ((cur->tok_value == K_INSERT) &&
         ((cur->next != NULL) && (cur->next->tok_value == K_INTO)))
	{
		printf("INSERT statement\n");
		cur_cmd = INSERT;
		cur = cur->next->next;   // position cur at the table IDENT for sem_insert(...)
	}
	else if ((cur->tok_value == K_DELETE) &&
         ((cur->next != NULL) && (cur->next->tok_value == K_FROM)))
	{
		printf("DELETE statement\n");
		cur_cmd = DELETE;
		cur = cur->next->next;   // position cur at the table IDENT for sem_delete(...)
	}
	else if (cur->tok_value == K_UPDATE)
	{
		printf("UPDATE statement\n");
		cur_cmd = UPDATE;
		cur = cur->next;         // position cur at the table IDENT for sem_update(...)
	}
	else if (cur->tok_value == K_SELECT)
	{
		printf("SELECT statement\n");
		cur_cmd = SELECT;
		cur = cur->next;         // position cur at S_STAR (for sem_select_stmt(...))
	}
	else
  {
		printf("Invalid statement\n");
		rc = cur_cmd;
	}

	if (cur_cmd != INVALID_STATEMENT)
	{
		switch(cur_cmd)
		{
			case CREATE_TABLE:
						rc = sem_create_table(cur);
						break;
			case DROP_TABLE:
						rc = sem_drop_table(cur);
						break;
			case LIST_TABLE:
						rc = sem_list_tables();
						break;
			case LIST_SCHEMA:
						rc = sem_list_schema(cur);
						break;
		case INSERT:
					rc = sem_insert(cur);
					break;
		case DELETE:
					rc = sem_delete(cur);
					break;
		case UPDATE:
					rc = sem_update(cur);
					break;
		case SELECT:
					rc = sem_select_stmt(cur);
					break;
		default:
				; /* no action */
		}
	}
	
	return rc;
}

int sem_create_table(token_list *t_list)
{
	int rc = 0;
	token_list *cur;
	tpd_entry tab_entry;
	tpd_entry *new_entry = NULL;
	bool column_done = false;
	int cur_id = 0;
	cd_entry	col_entry[MAX_NUM_COL];


	memset(&tab_entry, '\0', sizeof(tpd_entry));
	cur = t_list;
	if ((cur->tok_class != keyword) &&
		  (cur->tok_class != identifier) &&
			(cur->tok_class != type_name))
	{
		// Error
		rc = INVALID_TABLE_NAME;
		cur->tok_value = INVALID;
	}
	else
	{
		if ((new_entry = get_tpd_from_list(cur->tok_string)) != NULL)
		{
			rc = DUPLICATE_TABLE_NAME;
			cur->tok_value = INVALID;
		}
		else
		{
			strcpy(tab_entry.table_name, cur->tok_string);
			cur = cur->next;
			if (cur->tok_value != S_LEFT_PAREN)
			{
				//Error
				rc = INVALID_TABLE_DEFINITION;
				cur->tok_value = INVALID;
			}
			else
			{
				memset(&col_entry, '\0', (MAX_NUM_COL * sizeof(cd_entry)));

				/* Now build a set of column entries */
				cur = cur->next;
				do
				{
					if ((cur->tok_class != keyword) &&
							(cur->tok_class != identifier) &&
							(cur->tok_class != type_name))
					{
						// Error
						rc = INVALID_COLUMN_NAME;
						cur->tok_value = INVALID;
					}
					else
					{
						int i;
						for(i = 0; i < cur_id; i++)
						{
              /* make column name case sensitive */
							if (strcmp(col_entry[i].col_name, cur->tok_string)==0)
							{
								rc = DUPLICATE_COLUMN_NAME;
								cur->tok_value = INVALID;
								break;
							}
						}

						if (!rc)
						{
							strcpy(col_entry[cur_id].col_name, cur->tok_string);
							col_entry[cur_id].col_id = cur_id;
							col_entry[cur_id].not_null = false;    /* set default */

							cur = cur->next;
							if (cur->tok_class != type_name)
							{
								// Error
								rc = INVALID_TYPE_NAME;
								cur->tok_value = INVALID;
							}
							else
							{
                /* Set the column type here, int or char */
								col_entry[cur_id].col_type = cur->tok_value;
								cur = cur->next;
		
								if (col_entry[cur_id].col_type == T_INT)
								{
									if ((cur->tok_value != S_COMMA) &&
										  (cur->tok_value != K_NOT) &&
										  (cur->tok_value != S_RIGHT_PAREN))
									{
										rc = INVALID_COLUMN_DEFINITION;
										cur->tok_value = INVALID;
									}
								  else
									{
										col_entry[cur_id].col_len = sizeof(int);
										
										if ((cur->tok_value == K_NOT) &&
											  (cur->next->tok_value != K_NULL))
										{
											rc = INVALID_COLUMN_DEFINITION;
											cur->tok_value = INVALID;
										}	
										else if ((cur->tok_value == K_NOT) &&
											    (cur->next->tok_value == K_NULL))
										{					
											col_entry[cur_id].not_null = true;
											cur = cur->next->next;
										}
	
										if (!rc)
										{
											/* I must have either a comma or right paren */
											if ((cur->tok_value != S_RIGHT_PAREN) &&
												  (cur->tok_value != S_COMMA))
											{
												rc = INVALID_COLUMN_DEFINITION;
												cur->tok_value = INVALID;
											}
											else
		                  {
												if (cur->tok_value == S_RIGHT_PAREN)
												{
 													column_done = true;
												}
												cur = cur->next;
											}
										}
									}
								}   // end of T_INT processing
								else
								{
									// It must be char() or varchar() 
									if (cur->tok_value != S_LEFT_PAREN)
									{
										rc = INVALID_COLUMN_DEFINITION;
										cur->tok_value = INVALID;
									}
									else
									{
										/* Enter char(n) processing */
										cur = cur->next;
		
										if (cur->tok_value != INT_LITERAL)
										{
											rc = INVALID_COLUMN_LENGTH;
											cur->tok_value = INVALID;
										}
										else
										{
											/* Got a valid integer - convert */
											col_entry[cur_id].col_len = atoi(cur->tok_string);
											cur = cur->next;
											
											if (cur->tok_value != S_RIGHT_PAREN)
											{
												rc = INVALID_COLUMN_DEFINITION;
												cur->tok_value = INVALID;
											}
											else
											{
												cur = cur->next;
						
												if ((cur->tok_value != S_COMMA) &&
														(cur->tok_value != K_NOT) &&
														(cur->tok_value != S_RIGHT_PAREN))
												{
													rc = INVALID_COLUMN_DEFINITION;
													cur->tok_value = INVALID;
												}
												else
												{
													if ((cur->tok_value == K_NOT) &&
														  (cur->next->tok_value != K_NULL))
													{
														rc = INVALID_COLUMN_DEFINITION;
														cur->tok_value = INVALID;
													}
													else if ((cur->tok_value == K_NOT) &&
																	 (cur->next->tok_value == K_NULL))
													{					
														col_entry[cur_id].not_null = true;
														cur = cur->next->next;
													}
		
													if (!rc)
													{
														/* I must have either a comma or right paren */
														if ((cur->tok_value != S_RIGHT_PAREN) &&															  (cur->tok_value != S_COMMA))
														{
															rc = INVALID_COLUMN_DEFINITION;
															cur->tok_value = INVALID;
														}
														else
													  {
															if (cur->tok_value == S_RIGHT_PAREN)
															{
																column_done = true;
															}
															cur = cur->next;
														}
													}
												}
											}
										}	/* end char(n) processing */
									}
								} /* end char processing */
							}
						}  // duplicate column name
					} // invalid column name

					/* If rc=0, then get ready for the next column */
					if (!rc)
					{
						cur_id++;
					}

				} while ((rc == 0) && (!column_done));
	
				if ((column_done) && (cur->tok_value != EOC))
				{
					rc = INVALID_TABLE_DEFINITION;
					cur->tok_value = INVALID;
				}

				if (!rc)
				{
					/* Now finished building tpd and add it to the tpd list */
					tab_entry.num_columns = cur_id;
					tab_entry.tpd_size = sizeof(tpd_entry) + 
															 sizeof(cd_entry) *	tab_entry.num_columns;
				  tab_entry.cd_offset = sizeof(tpd_entry);
					new_entry = (tpd_entry*)calloc(1, tab_entry.tpd_size);

					if (new_entry == NULL)
					{
						rc = MEMORY_ERROR;
					}
					else
					{
						memcpy((void*)new_entry,
							     (void*)&tab_entry,
									 sizeof(tpd_entry));
		
						memcpy((void*)((char*)new_entry + sizeof(tpd_entry)),
									 (void*)col_entry,
									 sizeof(cd_entry) * tab_entry.num_columns);
	
						rc = add_tpd_to_list(new_entry);

						/* Create the empty <table>.tab file using the in-memory TPD we just built.
						IMPORTANT: do this BEFORE freeing new_entry. */
						if (rc == 0)
						{
						int rc2 = create_table_file(new_entry);
						if (rc2 != 0)
						{
							printf("ERROR: failed to create data file for table %s (rc=%d)\n",
								tab_entry.table_name, rc2);
							// If you want, you could also set rc = rc2 here.
							return rc2;
						}
						}

						/* Now it’s safe to free the temp buffer */
						free(new_entry);
					}
				}
			}
		}
	}
  return rc;
}

int sem_drop_table(token_list *t_list)
{
  int rc = 0;
  token_list *cur;
  tpd_entry *tab_entry = NULL;

  cur = t_list;
  if ((cur->tok_class != keyword) &&
      (cur->tok_class != identifier) &&
      (cur->tok_class != type_name))
  {
    // Error
    rc = INVALID_TABLE_NAME;
    cur->tok_value = INVALID;
  }
  else
  {
    if (cur->next->tok_value != EOC)
    {
      rc = INVALID_STATEMENT;
      cur->next->tok_value = INVALID;
    }
    else
    {
      if ((tab_entry = get_tpd_from_list(cur->tok_string)) == NULL)
      {
        rc = TABLE_NOT_EXIST;
        cur->tok_value = INVALID;
      }
      else
      {
        /* Found a valid tpd, drop it from tpd list */
        rc = drop_tpd_from_list(cur->tok_string);

        /* NEW: If catalog drop succeeded, remove the table's data file */
        if (rc == 0)
        {
          int frc = drop_table_file(cur->tok_string);   // returns 0 on success
          if (frc != 0)
          {
            // Keep original rc; just warn so behavior is unchanged
            printf("WARNING: unable to remove %s.tab (code=%d)\n", cur->tok_string, frc);
          }
        }
      }
    }
  }

  return rc;
}

int sem_list_tables()
{
	int rc = 0;
	int num_tables = g_tpd_list->num_tables;
	tpd_entry *cur = &(g_tpd_list->tpd_start);

	if (num_tables == 0)
	{
		printf("\nThere are currently no tables defined\n");
	}
	else
	{
		printf("\nTable List\n");
		printf("*****************\n");
		while (num_tables-- > 0)
		{
			printf("%s\n", cur->table_name);
			if (num_tables > 0)
			{
				cur = (tpd_entry*)((char*)cur + cur->tpd_size);
			}
		}
		printf("****** End ******\n");
	}

  return rc;
}

int sem_list_schema(token_list *t_list)
{
	int rc = 0;
	token_list *cur;
	tpd_entry *tab_entry = NULL;
	cd_entry  *col_entry = NULL;
	char tab_name[MAX_IDENT_LEN+1];
	char filename[MAX_IDENT_LEN+1];
	bool report = false;
	FILE *fhandle = NULL;
	int i = 0;

	cur = t_list;

	if (cur->tok_value != K_FOR)
  {
		rc = INVALID_STATEMENT;
		cur->tok_value = INVALID;
	}
	else
	{
		cur = cur->next;

		if ((cur->tok_class != keyword) &&
			  (cur->tok_class != identifier) &&
				(cur->tok_class != type_name))
		{
			// Error
			rc = INVALID_TABLE_NAME;
			cur->tok_value = INVALID;
		}
		else
		{
			memset(filename, '\0', MAX_IDENT_LEN+1);
			strcpy(tab_name, cur->tok_string);
			cur = cur->next;

			if (cur->tok_value != EOC)
			{
				if (cur->tok_value == K_TO)
				{
					cur = cur->next;
					
					if ((cur->tok_class != keyword) &&
						  (cur->tok_class != identifier) &&
							(cur->tok_class != type_name))
					{
						// Error
						rc = INVALID_REPORT_FILE_NAME;
						cur->tok_value = INVALID;
					}
					else
					{
						if (cur->next->tok_value != EOC)
						{
							rc = INVALID_STATEMENT;
							cur->next->tok_value = INVALID;
						}
						else
						{
							/* We have a valid file name */
							strcpy(filename, cur->tok_string);
							report = true;
						}
					}
				}
				else
				{ 
					/* Missing the TO keyword */
					rc = INVALID_STATEMENT;
					cur->tok_value = INVALID;
				}
			}

			if (!rc)
			{
				if ((tab_entry = get_tpd_from_list(tab_name)) == NULL)
				{
					rc = TABLE_NOT_EXIST;
					cur->tok_value = INVALID;
				}
				else
				{
					if (report)
					{
						if((fhandle = fopen(filename, "a+tc")) == NULL)
						{
							rc = FILE_OPEN_ERROR;
						}
					}

					if (!rc)
					{
						/* Find correct tpd, need to parse column and index information */

						/* First, write the tpd_entry information */
						printf("Table PD size            (tpd_size)    = %d\n", tab_entry->tpd_size);
						printf("Table Name               (table_name)  = %s\n", tab_entry->table_name);
						printf("Number of Columns        (num_columns) = %d\n", tab_entry->num_columns);
						printf("Column Descriptor Offset (cd_offset)   = %d\n", tab_entry->cd_offset);
            printf("Table PD Flags           (tpd_flags)   = %d\n\n", tab_entry->tpd_flags); 

						if (report)
						{
							fprintf(fhandle, "Table PD size            (tpd_size)    = %d\n", tab_entry->tpd_size);
							fprintf(fhandle, "Table Name               (table_name)  = %s\n", tab_entry->table_name);
							fprintf(fhandle, "Number of Columns        (num_columns) = %d\n", tab_entry->num_columns);
							fprintf(fhandle, "Column Descriptor Offset (cd_offset)   = %d\n", tab_entry->cd_offset);
              fprintf(fhandle, "Table PD Flags           (tpd_flags)   = %d\n\n", tab_entry->tpd_flags); 
						}

						/* Next, write the cd_entry information */
						for(i = 0, col_entry = (cd_entry*)((char*)tab_entry + tab_entry->cd_offset);
								i < tab_entry->num_columns; i++, col_entry++)
						{
							printf("Column Name   (col_name) = %s\n", col_entry->col_name);
							printf("Column Id     (col_id)   = %d\n", col_entry->col_id);
							printf("Column Type   (col_type) = %d\n", col_entry->col_type);
							printf("Column Length (col_len)  = %d\n", col_entry->col_len);
							printf("Not Null flag (not_null) = %d\n\n", col_entry->not_null);

							if (report)
							{
								fprintf(fhandle, "Column Name   (col_name) = %s\n", col_entry->col_name);
								fprintf(fhandle, "Column Id     (col_id)   = %d\n", col_entry->col_id);
								fprintf(fhandle, "Column Type   (col_type) = %d\n", col_entry->col_type);
								fprintf(fhandle, "Column Length (col_len)  = %d\n", col_entry->col_len);
								fprintf(fhandle, "Not Null Flag (not_null) = %d\n\n", col_entry->not_null);
							}
						}
	
						if (report)
						{
							fflush(fhandle);
							fclose(fhandle);
						}
					} // File open error							
				} // Table not exist
			} // no semantic errors
		} // Invalid table name
	} // Invalid statement

  return rc;
}

int initialize_tpd_list()
{
	int rc = 0;
	FILE *fhandle = NULL;
//	struct _stat file_stat;
	struct stat file_stat;

  /* Open for read */
  if((fhandle = fopen("dbfile.bin", "rbc")) == NULL)
	{
		if((fhandle = fopen("dbfile.bin", "wbc")) == NULL)
		{
			rc = FILE_OPEN_ERROR;
		}
    else
		{
			g_tpd_list = NULL;
			g_tpd_list = (tpd_list*)calloc(1, sizeof(tpd_list));
			
			if (!g_tpd_list)
			{
				rc = MEMORY_ERROR;
			}
			else
			{
				g_tpd_list->list_size = sizeof(tpd_list);
				fwrite(g_tpd_list, sizeof(tpd_list), 1, fhandle);
				fflush(fhandle);
				fclose(fhandle);
			}
		}
	}
	else
	{
		/* There is a valid dbfile.bin file - get file size */
//		_fstat(_fileno(fhandle), &file_stat);
		fstat(fileno(fhandle), &file_stat);
		printf("dbfile.bin size = %lld\n", (long long)file_stat.st_size);

		g_tpd_list = (tpd_list*)calloc(1, file_stat.st_size);

		if (!g_tpd_list)
		{
			rc = MEMORY_ERROR;
		}
		else
		{
			fread(g_tpd_list, file_stat.st_size, 1, fhandle);
			fflush(fhandle);
			fclose(fhandle);

			if (g_tpd_list->list_size != file_stat.st_size)
			{
				rc = DBFILE_CORRUPTION;
			}

		}
	}
    
	return rc;
}
	
int reload_tpd_list()
{
	int rc = 0;
	FILE *fhandle = NULL;
	struct stat file_stat;

	if ((fhandle = fopen("dbfile.bin", "rbc")) == NULL)
	{
		return FILE_OPEN_ERROR;
	}

	if (fstat(fileno(fhandle), &file_stat) != 0)
	{
		fclose(fhandle);
		return FILE_OPEN_ERROR;
	}

	tpd_list *new_list = (tpd_list*)calloc(1, file_stat.st_size);
	if (!new_list)
	{
		fclose(fhandle);
		return MEMORY_ERROR;
	}

	size_t read_count = fread(new_list, file_stat.st_size, 1, fhandle);
	fclose(fhandle);

	if (read_count != 1)
	{
		free(new_list);
		return FILE_OPEN_ERROR;
	}

	if (new_list->list_size != file_stat.st_size)
	{
		free(new_list);
		return DBFILE_CORRUPTION;
	}

	if (g_tpd_list)
	{
		free(g_tpd_list);
	}

	g_tpd_list = new_list;
	return rc;
}

int add_tpd_to_list(tpd_entry *tpd)
{
	if (!tpd || !g_tpd_list)
	{
		return DBFILE_CORRUPTION;
	}

	int old_num_tables = g_tpd_list->num_tables;
	int old_size = g_tpd_list->list_size;
	int grow = (old_num_tables == 0) ? (tpd->tpd_size - sizeof(tpd_entry)) : tpd->tpd_size;
	int new_size = old_size + grow;

	tpd_list *new_list = (tpd_list*)calloc(1, new_size);
	if (!new_list)
	{
		return MEMORY_ERROR;
	}

	memcpy(new_list, g_tpd_list, old_size);
	new_list->num_tables = old_num_tables + 1;
	new_list->list_size = new_size;

	if (old_num_tables == 0)
	{
		memcpy(&(new_list->tpd_start), tpd, tpd->tpd_size);
	}
	else
	{
		char *dest = ((char*)new_list) + old_size;
		memcpy(dest, tpd, tpd->tpd_size);
	}

	FILE *fhandle = fopen("dbfile.bin", "wbc");
	if (fhandle == NULL)
	{
		free(new_list);
		return FILE_OPEN_ERROR;
	}

	size_t written = fwrite(new_list, new_list->list_size, 1, fhandle);
	fflush(fhandle);
	fclose(fhandle);

	if (written != 1)
	{
		free(new_list);
		return FILE_OPEN_ERROR;
	}

	free(g_tpd_list);
	g_tpd_list = new_list;

	int reload_rc = reload_tpd_list();
	if (reload_rc != 0)
	{
		return reload_rc;
	}

	return 0;
}

int drop_tpd_from_list(char *tabname)
{
	int rc = 0;
	if (!tabname || !g_tpd_list || g_tpd_list->num_tables == 0)
	{
		return INVALID_TABLE_NAME;
	}

	int num_tables = g_tpd_list->num_tables;
	std::vector<tpd_entry*> keep;
	keep.reserve(num_tables);

	tpd_entry *cur = &(g_tpd_list->tpd_start);
	bool found = false;

	for (int i = 0; i < num_tables; ++i)
	{
		if (strcasecmp(cur->table_name, tabname) == 0)
		{
			found = true;
		}
		else
		{
			keep.push_back(cur);
		}

		if (i + 1 < num_tables)
		{
			cur = (tpd_entry*)((char*)cur + cur->tpd_size);
		}
	}

	if (!found)
	{
		return INVALID_TABLE_NAME;
	}

	int new_size = sizeof(tpd_list);
	if (!keep.empty())
	{
		new_size = sizeof(tpd_list) - sizeof(tpd_entry) + keep[0]->tpd_size;
		for (size_t i = 1; i < keep.size(); ++i)
		{
			new_size += keep[i]->tpd_size;
		}
	}

	tpd_list *new_list = (tpd_list*)calloc(1, new_size);
	if (!new_list)
	{
		return MEMORY_ERROR;
	}

	new_list->db_flags = g_tpd_list->db_flags;
	new_list->num_tables = static_cast<int>(keep.size());
	new_list->list_size = new_size;

	if (!keep.empty())
	{
		memcpy(&(new_list->tpd_start), keep[0], keep[0]->tpd_size);
		int offset = sizeof(tpd_list) - sizeof(tpd_entry) + keep[0]->tpd_size;

		for (size_t i = 1; i < keep.size(); ++i)
		{
			memcpy(((char*)new_list) + offset, keep[i], keep[i]->tpd_size);
			offset += keep[i]->tpd_size;
		}
	}

	FILE *fhandle = fopen("dbfile.bin", "wbc");
	if (fhandle == NULL)
	{
		free(new_list);
		return FILE_OPEN_ERROR;
	}

	size_t written = fwrite(new_list, new_list->list_size, 1, fhandle);
	fflush(fhandle);
	fclose(fhandle);

	if (written != 1)
	{
		free(new_list);
		return FILE_OPEN_ERROR;
	}

	free(g_tpd_list);
	g_tpd_list = new_list;

	int reload_rc = reload_tpd_list();
	if (reload_rc != 0)
	{
		return reload_rc;
	}

	return rc;
}

tpd_entry* get_tpd_from_list(char *tabname)
{
	tpd_entry *tpd = NULL;
	tpd_entry *cur = &(g_tpd_list->tpd_start);
	int num_tables = g_tpd_list->num_tables;
	bool found = false;

	if (num_tables > 0)
	{
		while ((!found) && (num_tables-- > 0))
		{
			if (strcasecmp(cur->table_name, tabname) == 0)
			{
				/* found it */
				found = true;
				tpd = cur;
			}
			else
			{
				if (num_tables > 0)
				{
					cur = (tpd_entry*)((char*)cur + cur->tpd_size);
				}
			}
		}
	}

	return tpd;
}

// Calculate the record size for a given table
static inline int round4(int n) { return (n + 3) & ~3; }

int calc_record_size(const tpd_entry *tpd)
{
  if (!tpd) return -1;

  int total = 0;
  cd_entry *col = (cd_entry *)((char *)tpd + tpd->cd_offset);
  for (int i = 0; i < tpd->num_columns; ++i, ++col)
  {
    if (col->col_type == T_INT)
      total += 1 /*len*/ + 4;
    else if (col->col_type == T_CHAR)
      total += 1 /*len*/ + col->col_len;
    else
      return -2; // unexpected type in part 1
  }
  return round4(total);
}

// Create the table file on disk
#include <cstdio>
#include <cstring>
#include <string>

static std::string table_filename(const char *name)
{
  return std::string(name) + ".tab";
}

int create_table_file(const tpd_entry *tpd)
{
  if (!tpd) return -1;

  int rec_size = calc_record_size(tpd);
  if (rec_size <= 0) return -2;

  table_file_header hdr;
  std::memset(&hdr, 0, sizeof(hdr));
  hdr.file_size       = sizeof(table_file_header);  // header only (no rows yet)
  hdr.record_size     = rec_size;
  hdr.num_records     = 0;
  hdr.record_offset   = sizeof(table_file_header);
  hdr.file_header_flag= 0;
  hdr.tpd_ptr         = nullptr;                    // must be 0 on disk

  std::string path = table_filename(tpd->table_name);
  FILE *fp = std::fopen(path.c_str(), "wb");
  if (!fp) return -3;

  size_t wrote = std::fwrite(&hdr, 1, sizeof(hdr), fp);
  std::fclose(fp);

  return (wrote == sizeof(hdr)) ? 0 : -4;
}

int drop_table_file(const char *table_name)
{
  if (!table_name) return -1;
  std::string path = table_filename(table_name);
  return std::remove(path.c_str());   // 0 on success, non-zero on failure
}

int load_table_header(const char *table_name, table_file_header *out_hdr)
{
  if (!table_name || !out_hdr) return -1;
  std::string path = table_filename(table_name);
  FILE *fp = std::fopen(path.c_str(), "rb");
  if (!fp) return -2;

  size_t rd = std::fread(out_hdr, 1, sizeof(table_file_header), fp);
  std::fclose(fp);
  if (rd != sizeof(table_file_header)) return -3;

  // IMPORTANT: refresh tpd_ptr in memory; never rely on on-disk value.
  out_hdr->tpd_ptr = get_tpd_from_list((char*)table_name);
  return (out_hdr->tpd_ptr ? 0 : -4);
}

int save_table_header(const char *table_name, const table_file_header *hdr)
{
  if (!table_name || !hdr) return -1;
  std::string path = table_filename(table_name);
  FILE *fp = std::fopen(path.c_str(), "rb+");  // update in place
  if (!fp) return -2;

  table_file_header tmp = *hdr;
  tmp.tpd_ptr = nullptr; // MUST be 0 before writing to disk
  size_t wr = std::fwrite(&tmp, 1, sizeof(tmp), fp);
  std::fclose(fp);
  return (wr == sizeof(tmp)) ? 0 : -3;
}

long data_area_pos(const table_file_header *hdr, int row_index)
{
  if (!hdr || row_index < 0) return -1;
  return (long)hdr->record_offset + (long)row_index * hdr->record_size;
}

// Helper to write one column and advance cursor
static int write_int_col(unsigned char *&p, int value, bool is_null)
{
  *p++ = is_null ? 0 : 4;        // length tag
  if (!is_null) {                // 4-byte payload
    std::memcpy(p, &value, 4);
  }
  p += 4;                        // reserve 4 regardless (fixed)
  return 0;
}

static int write_char_col(unsigned char *&p, const char *s, int maxlen, bool is_null)
{
  if (is_null) {
    *p++ = 0;                    // length 0 for NULL
    std::memset(p, 0, maxlen);   // keep payload region zeroed
    p += maxlen;
    return 0;
  }
  int n = (int)std::strlen(s);
  if (n > maxlen) n = maxlen;    // truncate to column length
  *p++ = (unsigned char)n;       // length tag
  std::memcpy(p, s, n);
  if (n < maxlen) std::memset(p + n, 0, maxlen - n);
  p += maxlen;
  return 0;
}

int pack_row_from_values(const tpd_entry *tpd,
                         const char **values,
                         unsigned char *out_record, int out_len)
{
  if (!tpd || !values || !out_record) return -1;

  int rec_size = calc_record_size(tpd);
  if (out_len < rec_size) return -2;

  std::memset(out_record, 0, out_len);
  unsigned char *p = out_record;

  cd_entry *col = (cd_entry *)((char *)tpd + tpd->cd_offset);
  for (int i = 0; i < tpd->num_columns; ++i, ++col)
  {
    const char *v = values[i];
    bool is_null = (v == nullptr) || (std::strcmp(v, "NULL") == 0);

    if (is_null && col->not_null)
    {
      return INVALID_COLUMN_DEFINITION;
    }

    if (col->col_type == T_INT)
    {
      int ival = 0;
      if (!is_null) ival = std::atoi(v);     // simple conversion for Part 1
      write_int_col(p, ival, is_null);
    }
    else if (col->col_type == T_CHAR)
    {
      write_char_col(p, is_null ? "" : v, col->col_len, is_null);
    }
    else return -3;
  }

  // pad to 4-byte boundary already guaranteed by rec_size
  return 0;
}

int unpack_row_to_strings(const tpd_entry *tpd,
                          const unsigned char *record,
                          char **out_strings, int out_cap_each)
{
  if (!tpd || !record || !out_strings) return -1;

  const unsigned char *p = record;
  cd_entry *col = (cd_entry *)((char *)tpd + tpd->cd_offset);

  for (int i = 0; i < tpd->num_columns; ++i, ++col)
  {
    unsigned char len = *p++;
    if (col->col_type == T_INT)
    {
      if (len == 0) {
        std::snprintf(out_strings[i], out_cap_each, "NULL");
      } else {
        int ival = 0;
        std::memcpy(&ival, p, 4);
        std::snprintf(out_strings[i], out_cap_each, "%d", ival);
      }
      p += 4;
    }
    else if (col->col_type == T_CHAR)
    {
      if (len == 0) {
        std::snprintf(out_strings[i], out_cap_each, "NULL");
        std::memset((void*)p, 0, col->col_len); // no-op on const p, fine to skip
      } else {
        int n = (len <= col->col_len) ? len : col->col_len;
        int copy = (n < out_cap_each-1) ? n : (out_cap_each-1);
        std::memcpy(out_strings[i], p, copy);
        out_strings[i][copy] = '\0';
      }
      p += col->col_len;
    }
    else return -2;
  }
  return 0;
}


int append_record(const char *table_name, const unsigned char *record, int rec_len)
{
  if (!table_name || !record || rec_len <= 0) return -1;

  // 1) Read header and verify record size
  table_file_header hdr;
  int rc = load_table_header(table_name, &hdr);
  if (rc != 0) return -2;

  if (rec_len != hdr.record_size) return -3;

  // 2) Open data file for update
  std::string path = table_filename(table_name);
  FILE *fp = std::fopen(path.c_str(), "rb+");
  if (!fp) return -4;

  // 3) Seek to end-of-data (offset for the next row)
  long pos = data_area_pos(&hdr, hdr.num_records);
  if (pos < 0) { std::fclose(fp); return -5; }

  if (std::fseek(fp, pos, SEEK_SET) != 0) { std::fclose(fp); return -6; }

  // 4) Write the new record bytes
  size_t wr = std::fwrite(record, 1, rec_len, fp);
  std::fflush(fp);
  std::fclose(fp);
  if (wr != (size_t)rec_len) return -7;

  // 5) Bump count and file size, then persist header
  hdr.num_records += 1;
  hdr.file_size = hdr.record_offset + hdr.num_records * hdr.record_size;

  rc = save_table_header(table_name, &hdr);
  if (rc != 0) return -8;

  return 0;
}

// Insert a new row into the table from string values
int insert_values(const char *table_name, const char **values, int value_count)
{
  if (!table_name || !values || value_count <= 0) return -1;

  // Get the TPD from memory; if not found, refresh the catalog ONCE.
  tpd_entry *tpd = get_tpd_from_list((char*)table_name);
  if (!tpd) {
    initialize_tpd_list();
    tpd = get_tpd_from_list((char*)table_name);
    if (!tpd) return TABLE_NOT_EXIST;
  }

  // 1) Load header (with fallback) and validate column count
  table_file_header hdr;
  int rc = load_table_header(table_name, &hdr);
  if (rc != 0) {
    // Fallback: read raw header directly, then inject the live tpd pointer
    std::string path = std::string(table_name) + ".tab";
    FILE *fp = std::fopen(path.c_str(), "rb");
    if (!fp) return -2; // header load failure (file open)
    size_t rd = std::fread(&hdr, 1, sizeof(table_file_header), fp);
    std::fclose(fp);
    if (rd != sizeof(table_file_header)) return -2; // header load failure (short read)
  }
  hdr.tpd_ptr = tpd;  // ensure live pointer is set

  if (value_count != tpd->num_columns) return -3;

  // 2) Pack record
  int rec_size = calc_record_size(tpd);
  std::vector<unsigned char> rec(rec_size);
  rc = pack_row_from_values(tpd, values, rec.data(), rec_size);
  if (rc != 0) return rc;

  // 3) Append to file and update header
  rc = append_record(table_name, rec.data(), rec_size);
  if (rc != 0) return rc;

  return 0;
}

// Select and print all records from the table
// Select and print all records from the table
int select_all(const char *table_name)
{
  if (!table_name) return -1;

  // Get the TPD; if not found, refresh once
  tpd_entry *tpd = get_tpd_from_list((char*)table_name);
  if (!tpd) {
    initialize_tpd_list();
    tpd = get_tpd_from_list((char*)table_name);
    if (!tpd) return TABLE_NOT_EXIST;
  }

  table_file_header hdr;
  int rc = load_table_header(table_name, &hdr);
  if (rc != 0) {
    // Fallback: read raw header directly, then inject the live tpd pointer
    std::string path = std::string(table_name) + ".tab";
    FILE *fp = std::fopen(path.c_str(), "rb");
    if (!fp) return -4; // keep your earlier convention for fopen failure here
    size_t rd = std::fread(&hdr, 1, sizeof(table_file_header), fp);
    std::fclose(fp);
    if (rd != sizeof(table_file_header)) return -4;
  }
  hdr.tpd_ptr = tpd; // ensure live pointer is set

  cd_entry *col = (cd_entry *)((char *)tpd + tpd->cd_offset);
  
  // Open file and read all records first to calculate column widths
  std::string path = table_filename(table_name);
  FILE *fp = std::fopen(path.c_str(), "rb");
  if (!fp) return -4;

  // Skip header
  if (std::fseek(fp, hdr.record_offset, SEEK_SET) != 0) { std::fclose(fp); return -5; }

  std::vector<unsigned char> rec(hdr.record_size);
  std::vector<std::string> bufs(tpd->num_columns);
  std::vector<char*> outs(tpd->num_columns);
  const int OUT_CAP = 512;
  for (int i = 0; i < tpd->num_columns; ++i) {
    bufs[i].assign(OUT_CAP, '\0');
    outs[i] = &bufs[i][0];
  }

  // Calculate column widths: start with column name lengths
  std::vector<int> col_widths(tpd->num_columns);
  for (int i = 0; i < tpd->num_columns; ++i) {
    col_widths[i] = (int)strlen(col[i].col_name);
  }

  // Read all records and update widths based on data
  std::vector<std::vector<std::string> > all_rows;
  for (int r = 0; r < hdr.num_records; ++r)
  {
    size_t rd = std::fread(rec.data(), 1, hdr.record_size, fp);
    if (rd != (size_t)hdr.record_size) { std::fclose(fp); return -6; }

    rc = unpack_row_to_strings(tpd, rec.data(), outs.data(), OUT_CAP);
    if (rc != 0) { std::fclose(fp); return -7; }

    std::vector<std::string> row(tpd->num_columns);
    for (int c = 0; c < tpd->num_columns; ++c) {
      row[c] = std::string(outs[c]);
      int len = (int)row[c].length();
      if (len > col_widths[c]) col_widths[c] = len;
    }
    all_rows.push_back(row);
  }
  std::fclose(fp);

  // Print header line: column names with proper spacing (all left-aligned)
  for (int i = 0; i < tpd->num_columns; ++i)
  {
    printf("%-*s", col_widths[i], col[i].col_name);
    if (i + 1 < tpd->num_columns) printf(" ");
  }
  printf("\n");

  // Print separator line
  for (int i = 0; i < tpd->num_columns; ++i)
  {
    for (int j = 0; j < col_widths[i]; ++j) printf("-");
    if (i + 1 < tpd->num_columns) printf(" ");
  }
  printf(" \n");  // trailing space before newline

  // Print data rows
  for (size_t r = 0; r < all_rows.size(); ++r)
  {
    for (int c = 0; c < tpd->num_columns; ++c)
    {
      if (col[c].col_type == T_INT) {
        // Right-align integer columns
        printf("%*s", col_widths[c], all_rows[r][c].c_str());
      } else {
        // Left-align string columns
        printf("%-*s", col_widths[c], all_rows[r][c].c_str());
      }
      if (c + 1 < tpd->num_columns) printf(" ");
    }
    printf("\n");
  }

  // Print record count
  printf("\n %d record(s) selected.\n", hdr.num_records);

  return 0;
}

// Select and print natural join of two tables
int select_natural_join(const char *table1_name, const char *table2_name)
{
  if (!table1_name || !table2_name) return -1;

  // Get TPDs for both tables
  tpd_entry *tpd1 = get_tpd_from_list((char*)table1_name);
  if (!tpd1) {
    initialize_tpd_list();
    tpd1 = get_tpd_from_list((char*)table1_name);
    if (!tpd1) return TABLE_NOT_EXIST;
  }

  tpd_entry *tpd2 = get_tpd_from_list((char*)table2_name);
  if (!tpd2) {
    initialize_tpd_list();
    tpd2 = get_tpd_from_list((char*)table2_name);
    if (!tpd2) return TABLE_NOT_EXIST;
  }

  // Load headers for both tables
  table_file_header hdr1, hdr2;
  int rc = load_table_header(table1_name, &hdr1);
  if (rc != 0) {
    std::string path = std::string(table1_name) + ".tab";
    FILE *fp = std::fopen(path.c_str(), "rb");
    if (!fp) return -2;
    size_t rd = std::fread(&hdr1, 1, sizeof(table_file_header), fp);
    std::fclose(fp);
    if (rd != sizeof(table_file_header)) return -2;
  }
  hdr1.tpd_ptr = tpd1;

  rc = load_table_header(table2_name, &hdr2);
  if (rc != 0) {
    std::string path = std::string(table2_name) + ".tab";
    FILE *fp = std::fopen(path.c_str(), "rb");
    if (!fp) return -2;
    size_t rd = std::fread(&hdr2, 1, sizeof(table_file_header), fp);
    std::fclose(fp);
    if (rd != sizeof(table_file_header)) return -2;
  }
  hdr2.tpd_ptr = tpd2;

  // Find common columns
  cd_entry *col1 = (cd_entry *)((char *)tpd1 + tpd1->cd_offset);
  cd_entry *col2 = (cd_entry *)((char *)tpd2 + tpd2->cd_offset);

  std::vector<int> common_col1_idx, common_col2_idx;
  std::vector<int> unique_col1_idx, unique_col2_idx;

  // Find common columns (case-sensitive match)
  for (int i = 0; i < tpd1->num_columns; ++i) {
    bool found = false;
    for (int j = 0; j < tpd2->num_columns; ++j) {
      if (strcmp(col1[i].col_name, col2[j].col_name) == 0) {
        // Common column - must have same type
        if (col1[i].col_type == col2[j].col_type) {
          common_col1_idx.push_back(i);
          common_col2_idx.push_back(j);
          found = true;
          break;
        }
      }
    }
    if (!found) {
      unique_col1_idx.push_back(i);
    }
  }

  // Find unique columns in table2
  for (int j = 0; j < tpd2->num_columns; ++j) {
    bool found = false;
    for (int i = 0; i < tpd1->num_columns; ++i) {
      if (strcmp(col1[i].col_name, col2[j].col_name) == 0 &&
          col1[i].col_type == col2[j].col_type) {
        found = true;
        break;
      }
    }
    if (!found) {
      unique_col2_idx.push_back(j);
    }
  }

  if (common_col1_idx.empty()) {
    // No common columns - return error or empty result
    printf("No common columns found for natural join\n");
    return -3;
  }

  // Read all records from both tables
  std::string path1 = table_filename(table1_name);
  FILE *fp1 = std::fopen(path1.c_str(), "rb");
  if (!fp1) return -4;
  if (std::fseek(fp1, hdr1.record_offset, SEEK_SET) != 0) { std::fclose(fp1); return -5; }

  std::string path2 = table_filename(table2_name);
  FILE *fp2 = std::fopen(path2.c_str(), "rb");
  if (!fp2) { std::fclose(fp1); return -4; }
  if (std::fseek(fp2, hdr2.record_offset, SEEK_SET) != 0) { std::fclose(fp1); std::fclose(fp2); return -5; }

  std::vector<unsigned char> rec1(hdr1.record_size);
  std::vector<unsigned char> rec2(hdr2.record_size);

  std::vector<std::string> bufs1(tpd1->num_columns);
  std::vector<char*> outs1(tpd1->num_columns);
  std::vector<std::string> bufs2(tpd2->num_columns);
  std::vector<char*> outs2(tpd2->num_columns);
  const int OUT_CAP = 512;
  for (int i = 0; i < tpd1->num_columns; ++i) {
    bufs1[i].assign(OUT_CAP, '\0');
    outs1[i] = &bufs1[i][0];
  }
  for (int i = 0; i < tpd2->num_columns; ++i) {
    bufs2[i].assign(OUT_CAP, '\0');
    outs2[i] = &bufs2[i][0];
  }

  // Read all rows from both tables
  std::vector<std::vector<std::string> > rows1, rows2;
  
  for (int r = 0; r < hdr1.num_records; ++r) {
    size_t rd = std::fread(rec1.data(), 1, hdr1.record_size, fp1);
    if (rd != (size_t)hdr1.record_size) { std::fclose(fp1); std::fclose(fp2); return -6; }
    rc = unpack_row_to_strings(tpd1, rec1.data(), outs1.data(), OUT_CAP);
    if (rc != 0) { std::fclose(fp1); std::fclose(fp2); return -7; }
    std::vector<std::string> row(tpd1->num_columns);
    for (int c = 0; c < tpd1->num_columns; ++c) {
      row[c] = std::string(outs1[c]);
    }
    rows1.push_back(row);
  }
  std::fclose(fp1);

  for (int r = 0; r < hdr2.num_records; ++r) {
    size_t rd = std::fread(rec2.data(), 1, hdr2.record_size, fp2);
    if (rd != (size_t)hdr2.record_size) { std::fclose(fp2); return -6; }
    rc = unpack_row_to_strings(tpd2, rec2.data(), outs2.data(), OUT_CAP);
    if (rc != 0) { std::fclose(fp2); return -7; }
    std::vector<std::string> row(tpd2->num_columns);
    for (int c = 0; c < tpd2->num_columns; ++c) {
      row[c] = std::string(outs2[c]);
    }
    rows2.push_back(row);
  }
  std::fclose(fp2);

  // Build result rows by joining on common columns
  std::vector<std::vector<std::string> > result_rows;
  int total_cols = (int)(common_col1_idx.size() + unique_col1_idx.size() + unique_col2_idx.size());

  for (size_t i = 0; i < rows1.size(); ++i) {
    for (size_t j = 0; j < rows2.size(); ++j) {
      // Check if all common columns match
      bool match = true;
      for (size_t k = 0; k < common_col1_idx.size(); ++k) {
        int idx1 = common_col1_idx[k];
        int idx2 = common_col2_idx[k];
        if (rows1[i][idx1] != rows2[j][idx2]) {
          match = false;
          break;
        }
      }
      
      if (match) {
        // Build joined row: common columns, unique from table1, unique from table2
        std::vector<std::string> joined_row(total_cols);
        int out_idx = 0;
        
        // Add common columns (from table1)
        for (size_t k = 0; k < common_col1_idx.size(); ++k) {
          joined_row[out_idx++] = rows1[i][common_col1_idx[k]];
        }
        
        // Add unique columns from table1
        for (size_t k = 0; k < unique_col1_idx.size(); ++k) {
          joined_row[out_idx++] = rows1[i][unique_col1_idx[k]];
        }
        
        // Add unique columns from table2
        for (size_t k = 0; k < unique_col2_idx.size(); ++k) {
          joined_row[out_idx++] = rows2[j][unique_col2_idx[k]];
        }
        
        result_rows.push_back(joined_row);
      }
    }
  }

  // Build column info for output
  std::vector<cd_entry> output_cols;
  std::vector<int> col_types;
  
  // Common columns
  for (size_t k = 0; k < common_col1_idx.size(); ++k) {
    output_cols.push_back(col1[common_col1_idx[k]]);
    col_types.push_back(col1[common_col1_idx[k]].col_type);
  }
  
  // Unique columns from table1
  for (size_t k = 0; k < unique_col1_idx.size(); ++k) {
    output_cols.push_back(col1[unique_col1_idx[k]]);
    col_types.push_back(col1[unique_col1_idx[k]].col_type);
  }
  
  // Unique columns from table2
  for (size_t k = 0; k < unique_col2_idx.size(); ++k) {
    output_cols.push_back(col2[unique_col2_idx[k]]);
    col_types.push_back(col2[unique_col2_idx[k]].col_type);
  }

  // Calculate column widths
  std::vector<int> col_widths(total_cols);
  for (int i = 0; i < total_cols; ++i) {
    col_widths[i] = (int)strlen(output_cols[i].col_name);
  }
  
  for (size_t r = 0; r < result_rows.size(); ++r) {
    for (int c = 0; c < total_cols; ++c) {
      int len = (int)result_rows[r][c].length();
      if (len > col_widths[c]) col_widths[c] = len;
    }
  }

  // Print header (all left-aligned)
  for (int i = 0; i < total_cols; ++i) {
    printf("%-*s", col_widths[i], output_cols[i].col_name);
    if (i + 1 < total_cols) printf(" ");
  }
  printf("\n");

  // Print separator
  for (int i = 0; i < total_cols; ++i) {
    for (int j = 0; j < col_widths[i]; ++j) printf("-");
    if (i + 1 < total_cols) printf(" ");
  }
  printf(" \n");  // trailing space before newline

  // Print data rows
  for (size_t r = 0; r < result_rows.size(); ++r) {
    for (int c = 0; c < total_cols; ++c) {
      if (col_types[c] == T_INT) {
        printf("%*s", col_widths[c], result_rows[r][c].c_str());
      } else {
        printf("%-*s", col_widths[c], result_rows[r][c].c_str());
      }
      if (c + 1 < total_cols) printf(" ");
    }
    printf("\n");
  }

  // Print record count
  printf("\n %d record(s) selected.\n", (int)result_rows.size());

  return 0;
}


// Helper to unpack a row into allocated strings
char **unpack_row_to_string_array(const unsigned char *record, const tpd_entry *tpd)
{
  if (!record || !tpd) return NULL;
  
  char **strings = (char**)malloc(tpd->num_columns * sizeof(char*));
  if (!strings) return NULL;
  
  for (int i = 0; i < tpd->num_columns; i++)
  {
    strings[i] = (char*)malloc(MAX_TOK_LEN);
    if (!strings[i])
    {
      for (int j = 0; j < i; j++) free(strings[j]);
      free(strings);
      return NULL;
    }
  }
  
  if (unpack_row_to_strings(tpd, record, strings, MAX_TOK_LEN) != 0)
  {
    for (int i = 0; i < tpd->num_columns; i++) free(strings[i]);
    free(strings);
    return NULL;
  }
  
  return strings;
}

// Execute enhanced SELECT with column list, WHERE, ORDER BY, aggregates
int execute_enhanced_select(const char *table_name, select_query *query, where_condition *where_cond)
{
  if (!table_name || !query) return -1;
  
  // Load table metadata
  tpd_entry *tpd = get_tpd_from_list((char*)table_name);
  if (!tpd) return TABLE_NOT_EXIST;
  
  table_file_header header;
  int rc = load_table_header(table_name, &header);
  if (rc != 0) return rc;
  
  cd_entry *columns = (cd_entry *)((char *)tpd + tpd->cd_offset);
  
  // Load all records
  std::vector<std::vector<char*>> all_rows;
  std::vector<char*> raw_records;
  
  char filename[MAX_IDENT_LEN + 5];
  snprintf(filename, sizeof(filename), "%s.tab", table_name);
  FILE *fptr = fopen(filename, "rb");
  if (!fptr) return FILE_OPEN_ERROR;
  
  fseek(fptr, header.record_offset, SEEK_SET);
  for (int i = 0; i < header.num_records; i++)
  {
    char *record = (char*)calloc(header.record_size, 1);
    if (fread(record, header.record_size, 1, fptr) != 1)
    {
      free(record);
      continue;
    }
    raw_records.push_back(record);
    
    char **row_strings = unpack_row_to_string_array((const unsigned char*)record, tpd);
    if (row_strings)
    {
      // Check WHERE condition
      if (evaluate_where_condition(where_cond, row_strings, tpd))
      {
        std::vector<char*> row_vec;
        for (int c = 0; c < tpd->num_columns; c++)
          row_vec.push_back(row_strings[c]);
        all_rows.push_back(row_vec);
      }
      else
      {
        for (int c = 0; c < tpd->num_columns; c++)
          free(row_strings[c]);
        free(row_strings);
      }
    }
  }
  fclose(fptr);
  
  // Handle aggregates
  if (query->agg_type != AGG_NONE)
  {
    int count = 0;
    long long sum = 0;
    bool valid_agg = true;
    
    // COUNT(*) special case
    if (query->agg_type == AGG_COUNT && query->agg_col_index == -1)
    {
      count = all_rows.size();
    }
    else if (query->agg_col_index >= 0 && query->agg_col_index < tpd->num_columns)
    {
      cd_entry *agg_col = &columns[query->agg_col_index];
      
      // Validate aggregate on INT columns only (for SUM/AVG)
      if ((query->agg_type == AGG_SUM || query->agg_type == AGG_AVG) && agg_col->col_type != T_INT)
      {
        // Free memory
        for (auto &row : all_rows)
        {
          for (char *str : row) free(str);
        }
        for (char *rec : raw_records) free(rec);
        return INVALID_AGGREGATE_COLUMN;
      }
      
      for (auto &row : all_rows)
      {
        if (strcmp(row[query->agg_col_index], "NULL") != 0)
        {
          if (query->agg_type == AGG_COUNT)
          {
            count++;
          }
          else if (query->agg_type == AGG_SUM || query->agg_type == AGG_AVG)
          {
            sum += atoi(row[query->agg_col_index]);
            count++;
          }
        }
      }
    }
    
    // Print aggregate result
    if (query->agg_type == AGG_SUM)
      printf("SUM(%s)\n----------\n%lld\n", query->agg_col_name, sum);
    else if (query->agg_type == AGG_AVG)
      printf("AVG(%s)\n----------\n%s\n", query->agg_col_name, count > 0 ? std::to_string(sum / count).c_str() : "NULL");
    else if (query->agg_type == AGG_COUNT)
    {
      if (query->agg_col_index == -1)
        printf("COUNT(*)\n----------\n%d\n", count);
      else
        printf("COUNT(%s)\n----------\n%d\n", query->agg_col_name, count);
    }
    
    // Free memory
    for (auto &row : all_rows)
    {
      for (char *str : row) free(str);
    }
    for (char *rec : raw_records) free(rec);
    return 0;
  }
  
  // ORDER BY sorting
  if (query->order_by_col_index >= 0)
  {
    int sort_col_idx = query->order_by_col_index;
    cd_entry *sort_col = &columns[sort_col_idx];
    bool descending = query->order_desc;
    
    std::sort(all_rows.begin(), all_rows.end(), 
      [sort_col_idx, sort_col, descending](const std::vector<char*> &a, const std::vector<char*> &b) {
        bool a_null = (strcmp(a[sort_col_idx], "NULL") == 0);
        bool b_null = (strcmp(b[sort_col_idx], "NULL") == 0);
        if (a_null && b_null) return false;
        if (a_null) return !descending;  // NULLs sort first ascending
        if (b_null) return descending;
        
        int cmp;
        if (sort_col->col_type == T_INT)
          cmp = atoi(a[sort_col_idx]) - atoi(b[sort_col_idx]);
        else
          cmp = strcmp(a[sort_col_idx], b[sort_col_idx]);
        
        return descending ? (cmp > 0) : (cmp < 0);
      });
  }
  
  // Calculate column widths for selected columns
  std::vector<int> col_widths;
  std::vector<int> selected_cols;
  
  if (query->col_type == SEL_STAR)
  {
    for (int i = 0; i < tpd->num_columns; i++)
      selected_cols.push_back(i);
  }
  else if (query->col_type == SEL_COLUMNS)
  {
    for (int i = 0; i < query->num_columns; i++)
      selected_cols.push_back(query->col_indices[i]);
  }
  
  for (int col_idx : selected_cols)
  {
    cd_entry *col = &columns[col_idx];
    int max_width = strlen(col->col_name);
    
    for (auto &row : all_rows)
    {
      int len = strlen(row[col_idx]);
      if (len > max_width) max_width = len;
    }
    
    col_widths.push_back(std::max(max_width, 4));
  }
  
  // Print header
  for (size_t i = 0; i < selected_cols.size(); i++)
  {
    cd_entry *col = &columns[selected_cols[i]];
    printf("%-*s", col_widths[i], col->col_name);
    if (i < selected_cols.size() - 1) printf(" ");
  }
  printf("\n");
  
  // Print separator
  for (size_t i = 0; i < selected_cols.size(); i++)
  {
    for (int j = 0; j < col_widths[i]; j++) printf("-");
    if (i < selected_cols.size() - 1) printf(" ");
  }
  printf(" \n");
  
  // Print data rows
  for (auto &row : all_rows)
  {
    for (size_t i = 0; i < selected_cols.size(); i++)
    {
      int col_idx = selected_cols[i];
      cd_entry *col = &columns[col_idx];
      
      if (col->col_type == T_INT)
        printf("%*s", col_widths[i], row[col_idx]);
      else
        printf("%-*s", col_widths[i], row[col_idx]);
      
      if (i < selected_cols.size() - 1) printf(" ");
    }
    printf("\n");
  }
  
  printf("\n %d record(s) selected.\n", (int)all_rows.size());
  
  // Free memory
  for (auto &row : all_rows)
  {
    for (char *str : row) free(str);
  }
  for (char *rec : raw_records) free(rec);
  
  return 0;
}

int sem_select_stmt(token_list *t_list)
{
  if (!t_list) return INVALID_STATEMENT;
  
  token_list *cur = t_list;
  select_query query;
  memset(&query, 0, sizeof(query));
  query.agg_type = AGG_NONE;
  query.agg_col_index = -1;
  query.order_by_col_index = -1;
  query.order_desc = false;
  
  // Parse column specification or aggregate or *
  if (cur->tok_value == S_STAR)
  {
    query.col_type = SEL_STAR;
    cur = cur->next;
  }
  else if (cur->tok_value == F_SUM || cur->tok_value == F_AVG || cur->tok_value == F_COUNT)
  {
    // Aggregate function
    query.col_type = SEL_AGGREGATE;
    
    if (cur->tok_value == F_SUM)
      query.agg_type = AGG_SUM;
    else if (cur->tok_value == F_AVG)
      query.agg_type = AGG_AVG;
    else if (cur->tok_value == F_COUNT)
      query.agg_type = AGG_COUNT;
    
    cur = cur->next;
    if (!cur || cur->tok_value != S_LEFT_PAREN) return INVALID_STATEMENT;
    
    cur = cur->next;
    if (!cur) return INVALID_STATEMENT;
    
    if (cur->tok_value == S_STAR)
    {
      // COUNT(*)
      if (query.agg_type != AGG_COUNT) return INVALID_STATEMENT;
      query.agg_col_index = -1;
      strcpy(query.agg_col_name, "*");
    }
    else if (cur->tok_class == identifier || cur->tok_class == keyword || cur->tok_class == type_name)
    {
      strcpy(query.agg_col_name, cur->tok_string);
    }
    else
    {
      return INVALID_COLUMN_NAME;
    }
    
    cur = cur->next;
    if (!cur || cur->tok_value != S_RIGHT_PAREN) return INVALID_STATEMENT;
    cur = cur->next;
  }
  else if (cur->tok_class == identifier || cur->tok_class == keyword || cur->tok_class == type_name)
  {
    // Column list
    query.col_type = SEL_COLUMNS;
    query.num_columns = 0;
    
    while (cur && (cur->tok_class == identifier || cur->tok_class == keyword || cur->tok_class == type_name))
    {
      if (query.num_columns >= MAX_NUM_COL) return TOO_MANY_COLUMNS;
      
      // Store column name temporarily (will resolve to index later)
      // For now, just count and advance
      query.num_columns++;
      
      cur = cur->next;
      if (!cur) return INVALID_STATEMENT;
      
      if (cur->tok_value == S_COMMA)
      {
        cur = cur->next;
        if (!cur) return INVALID_STATEMENT;
      }
      else
      {
        break;
      }
    }
    
    // Reset to parse column names properly
    cur = t_list;
    for (int i = 0; i < query.num_columns; i++)
    {
      // We'll resolve column names after we know the table
      cur = cur->next;
      if (cur && cur->tok_value == S_COMMA)
        cur = cur->next;
    }
  }
  else
  {
    return INVALID_STATEMENT;
  }
  
  // FROM keyword
  if (!cur || cur->tok_value != K_FROM) return INVALID_STATEMENT;
  
  cur = cur->next;
  if (!cur) return INVALID_STATEMENT;
  
  if (!(cur->tok_class == identifier || cur->tok_class == keyword || cur->tok_class == type_name))
    return INVALID_TABLE_NAME;
  
  const char *table_name = cur->tok_string;
  
  // Load table to resolve column names
  tpd_entry *tpd = get_tpd_from_list((char*)table_name);
  if (!tpd) return TABLE_NOT_EXIST;
  
  cd_entry *columns = (cd_entry *)((char *)tpd + tpd->cd_offset);
  
  // Resolve column names for SEL_COLUMNS
  if (query.col_type == SEL_COLUMNS)
  {
    token_list *col_token = t_list;
    for (int i = 0; i < query.num_columns; i++)
    {
      bool found = false;
      for (int c = 0; c < tpd->num_columns; c++)
      {
        if (strcasecmp(columns[c].col_name, col_token->tok_string) == 0)
        {
          query.col_indices[i] = c;
          found = true;
          break;
        }
      }
      if (!found)
      {
        printf("ERROR: Column '%s' not found in table '%s'\n", col_token->tok_string, table_name);
        return COLUMN_NOT_EXIST;
      }
      
      col_token = col_token->next;
      if (col_token && col_token->tok_value == S_COMMA)
        col_token = col_token->next;
    }
  }
  
  // Resolve aggregate column (if not COUNT(*))
  if (query.col_type == SEL_AGGREGATE && strcmp(query.agg_col_name, "*") != 0)
  {
    bool found = false;
    for (int c = 0; c < tpd->num_columns; c++)
    {
      if (strcasecmp(columns[c].col_name, query.agg_col_name) == 0)
      {
        query.agg_col_index = c;
        found = true;
        break;
      }
    }
    if (!found)
    {
      printf("ERROR: Column '%s' not found in table '%s'\n", query.agg_col_name, table_name);
      return COLUMN_NOT_EXIST;
    }
  }
  
  cur = cur->next;
  if (!cur) return INVALID_STATEMENT;
  
  // Check for NATURAL JOIN
  if (cur->tok_value == K_NATURAL)
  {
    // Only support SELECT * FROM t1 NATURAL JOIN t2 for now
    if (query.col_type != SEL_STAR)
    {
      printf("ERROR: NATURAL JOIN only supported with SELECT *\n");
      return INVALID_STATEMENT;
    }
    
    cur = cur->next;
    if (!cur || cur->tok_value != K_JOIN) return INVALID_STATEMENT;
    
    cur = cur->next;
    if (!cur) return INVALID_STATEMENT;
    
    if (!(cur->tok_class == identifier || cur->tok_class == keyword || cur->tok_class == type_name))
      return INVALID_TABLE_NAME;
    
    const char *table2_name = cur->tok_string;
    
    cur = cur->next;
    if (!cur || cur->tok_value != EOC) return INVALID_STATEMENT;
    
    return select_natural_join(table_name, table2_name);
  }
  
  // Parse WHERE clause
  where_condition *where_cond = NULL;
  if (cur->tok_value == K_WHERE)
  {
    cur = cur->next;
    where_cond = parse_where_clause(&cur, tpd);
    if (!where_cond && cur->tok_value != K_ORDER && cur->tok_value != EOC)
    {
      return INVALID_STATEMENT;
    }
  }
  
  // Parse ORDER BY
  if (cur && cur->tok_value == K_ORDER)
  {
    cur = cur->next;
    if (!cur || cur->tok_value != K_BY) return INVALID_STATEMENT;
    
    cur = cur->next;
    if (!cur) return INVALID_STATEMENT;
    
    if (!(cur->tok_class == identifier || cur->tok_class == keyword || cur->tok_class == type_name))
      return INVALID_COLUMN_NAME;
    
    strcpy(query.order_by_col, cur->tok_string);
    
    // Resolve column index
    bool found = false;
    for (int c = 0; c < tpd->num_columns; c++)
    {
      if (strcasecmp(columns[c].col_name, query.order_by_col) == 0)
      {
        query.order_by_col_index = c;
        found = true;
        break;
      }
    }
    if (!found)
    {
      printf("ERROR: Column '%s' not found in table '%s'\n", query.order_by_col, table_name);
      free_where_conditions(where_cond);
      return COLUMN_NOT_EXIST;
    }
    
    cur = cur->next;
    if (cur && cur->tok_value == K_DESC)
    {
      query.order_desc = true;
      cur = cur->next;
    }
  }
  
  if (!cur || cur->tok_value != EOC)
  {
    free_where_conditions(where_cond);
    return INVALID_STATEMENT;
  }
  
  int rc = execute_enhanced_select(table_name, &query, where_cond);
  free_where_conditions(where_cond);
  
  if (rc != 0)
  {
    printf("ERROR: select failed (rc=%d)\n", rc);
  }
  
  return rc;
}

// Helper: Free WHERE condition linked list
void free_where_conditions(where_condition *cond)
{
  where_condition *cur = cond;
  while (cur)
  {
    where_condition *next = cur->next;
    free(cur);
    cur = next;
  }
}

// Parse WHERE clause and return linked list of conditions
// Input: pointer to token list (updated to point after WHERE clause)
// Returns: pointer to first condition, or NULL on error
where_condition* parse_where_clause(token_list **cur_token, tpd_entry *tpd)
{
  if (!cur_token || !*cur_token || !tpd) return NULL;
  
  token_list *cur = *cur_token;
  where_condition *first_cond = NULL;
  where_condition *prev_cond = NULL;
  
  cd_entry *columns = (cd_entry *)((char *)tpd + tpd->cd_offset);
  
  while (cur && cur->tok_value != EOC && 
         cur->tok_value != K_ORDER && 
         cur->tok_value != K_NATURAL &&
         cur->tok_value != K_SET)
  {
    // Allocate new condition
    where_condition *cond = (where_condition *)calloc(1, sizeof(where_condition));
    if (!cond)
    {
      free_where_conditions(first_cond);
      return NULL;
    }
    
    // Link to previous condition
    if (prev_cond)
      prev_cond->next = cond;
    else
      first_cond = cond;
    
    cond->log_op = LOG_NONE;
    cond->next = NULL;
    cond->col_index = -1;
    
    // Parse column name
    if (!(cur->tok_class == identifier || cur->tok_class == keyword || cur->tok_class == type_name))
    {
      free_where_conditions(first_cond);
      return NULL;
    }
    
    strcpy(cond->col_name, cur->tok_string);
    
    // Find column index in TPD
    for (int i = 0; i < tpd->num_columns; i++)
    {
      if (strcasecmp(columns[i].col_name, cond->col_name) == 0)
      {
        cond->col_index = i;
        break;
      }
    }
    
    if (cond->col_index == -1)
    {
      // Column not found
      free_where_conditions(first_cond);
      return NULL;
    }
    
    cur = cur->next;
    if (!cur)
    {
      free_where_conditions(first_cond);
      return NULL;
    }
    
    // Parse operator
    if (cur->tok_value == S_EQUAL)
    {
      cond->op = OP_EQUAL;
      cur = cur->next;
      if (!cur || (cur->tok_class != constant && cur->tok_value != K_NULL))
      {
        free_where_conditions(first_cond);
        return NULL;
      }
      if (cur->tok_value == K_NULL)
        strcpy(cond->value, "NULL");
      else
        strcpy(cond->value, cur->tok_string);
      cur = cur->next;
    }
    else if (cur->tok_value == S_LESS)
    {
      cond->op = OP_LESS;
      cur = cur->next;
      if (!cur || (cur->tok_class != constant && cur->tok_value != K_NULL))
      {
        free_where_conditions(first_cond);
        return NULL;
      }
      if (cur->tok_value == K_NULL)
        strcpy(cond->value, "NULL");
      else
        strcpy(cond->value, cur->tok_string);
      cur = cur->next;
    }
    else if (cur->tok_value == S_GREATER)
    {
      cond->op = OP_GREATER;
      cur = cur->next;
      if (!cur || (cur->tok_class != constant && cur->tok_value != K_NULL))
      {
        free_where_conditions(first_cond);
        return NULL;
      }
      if (cur->tok_value == K_NULL)
        strcpy(cond->value, "NULL");
      else
        strcpy(cond->value, cur->tok_string);
      cur = cur->next;
    }
    else if (cur->tok_value == K_IS)
    {
      cur = cur->next;
      if (!cur)
      {
        free_where_conditions(first_cond);
        return NULL;
      }
      
      if (cur->tok_value == K_NULL)
      {
        cond->op = OP_IS_NULL;
        strcpy(cond->value, "NULL");
        cur = cur->next;
      }
      else if (cur->tok_value == K_NOT)
      {
        cur = cur->next;
        if (!cur || cur->tok_value != K_NULL)
        {
          free_where_conditions(first_cond);
          return NULL;
        }
        cond->op = OP_IS_NOT_NULL;
        strcpy(cond->value, "NULL");
        cur = cur->next;
      }
      else
      {
        free_where_conditions(first_cond);
        return NULL;
      }
    }
    else
    {
      free_where_conditions(first_cond);
      return NULL;
    }
    
    // Check for AND/OR
    if (cur && (cur->tok_value == K_AND || cur->tok_value == K_OR))
    {
      if (cur->tok_value == K_AND)
        cond->log_op = LOG_AND;
      else
        cond->log_op = LOG_OR;
      
      prev_cond = cond;
      cur = cur->next;
      // Loop continues to parse next condition
    }
    else
    {
      // No more conditions
      break;
    }
  }
  
  *cur_token = cur;
  return first_cond;
}

// Evaluate WHERE conditions against a row
// Returns true if row matches all conditions
bool evaluate_where_condition(where_condition *cond, char **row_data, tpd_entry *tpd)
{
  if (!cond) return true;  // No conditions means all rows match
  if (!row_data || !tpd) return false;
  
  cd_entry *columns = (cd_entry *)((char *)tpd + tpd->cd_offset);
  bool result = true;
  bool chain_result = true;
  logical_operator prev_log_op = LOG_NONE;
  
  while (cond)
  {
    bool cond_result = false;
    
    if (cond->col_index < 0 || cond->col_index >= tpd->num_columns)
    {
      return false;
    }
    
    char *col_value = row_data[cond->col_index];
    cd_entry *col_def = &columns[cond->col_index];
    bool is_null = (strcmp(col_value, "NULL") == 0);
    
    switch (cond->op)
    {
      case OP_IS_NULL:
        cond_result = is_null;
        break;
        
      case OP_IS_NOT_NULL:
        cond_result = !is_null;
        break;
        
      case OP_EQUAL:
        if (is_null || strcmp(cond->value, "NULL") == 0)
          cond_result = (is_null && strcmp(cond->value, "NULL") == 0);
        else if (col_def->col_type == T_INT)
          cond_result = (atoi(col_value) == atoi(cond->value));
        else
          cond_result = (strcmp(col_value, cond->value) == 0);
        break;
        
      case OP_LESS:
        if (!is_null && strcmp(cond->value, "NULL") != 0)
        {
          if (col_def->col_type == T_INT)
            cond_result = (atoi(col_value) < atoi(cond->value));
          else
            cond_result = (strcmp(col_value, cond->value) < 0);
        }
        break;
        
      case OP_GREATER:
        if (!is_null && strcmp(cond->value, "NULL") != 0)
        {
          if (col_def->col_type == T_INT)
            cond_result = (atoi(col_value) > atoi(cond->value));
          else
            cond_result = (strcmp(col_value, cond->value) > 0);
        }
        break;
    }
    
    // Combine with previous result using logical operator
    if (prev_log_op == LOG_NONE)
    {
      chain_result = cond_result;
    }
    else if (prev_log_op == LOG_AND)
    {
      chain_result = chain_result && cond_result;
    }
    else if (prev_log_op == LOG_OR)
    {
      chain_result = chain_result || cond_result;
    }
    
    prev_log_op = cond->log_op;
    cond = cond->next;
  }
  
  return chain_result;
}

// Delete records from table based on WHERE condition
int delete_records(const char *table_name, where_condition *where_cond)
{
  if (!table_name) return -1;
  
  // Get TPD
  tpd_entry *tpd = get_tpd_from_list((char*)table_name);
  if (!tpd) {
    initialize_tpd_list();
    tpd = get_tpd_from_list((char*)table_name);
    if (!tpd) return TABLE_NOT_EXIST;
  }
  
  // Load table header
  table_file_header hdr;
  int rc = load_table_header(table_name, &hdr);
  if (rc != 0) {
    std::string path = std::string(table_name) + ".tab";
    FILE *fp = std::fopen(path.c_str(), "rb");
    if (!fp) return -2;
    size_t rd = std::fread(&hdr, 1, sizeof(table_file_header), fp);
    std::fclose(fp);
    if (rd != sizeof(table_file_header)) return -2;
  }
  hdr.tpd_ptr = tpd;
  
  // Open table file and read all records
  std::string path = table_filename(table_name);
  FILE *fp = std::fopen(path.c_str(), "rb");
  if (!fp) return -3;
  
  if (std::fseek(fp, hdr.record_offset, SEEK_SET) != 0) { 
    std::fclose(fp); 
    return -4; 
  }
  
  std::vector<unsigned char> rec(hdr.record_size);
  std::vector<std::string> bufs(tpd->num_columns);
  std::vector<char*> outs(tpd->num_columns);
  const int OUT_CAP = 512;
  for (int i = 0; i < tpd->num_columns; ++i) {
    bufs[i].assign(OUT_CAP, '\0');
    outs[i] = &bufs[i][0];
  }
  
  // Read all rows and filter out deleted ones
  std::vector<std::vector<unsigned char> > kept_records;
  int deleted_count = 0;
  
  for (int r = 0; r < hdr.num_records; ++r)
  {
    size_t rd = std::fread(rec.data(), 1, hdr.record_size, fp);
    if (rd != (size_t)hdr.record_size) { 
      std::fclose(fp); 
      return -5; 
    }
    
    rc = unpack_row_to_strings(tpd, rec.data(), outs.data(), OUT_CAP);
    if (rc != 0) { 
      std::fclose(fp); 
      return -6; 
    }
    
    // Evaluate WHERE condition
    bool should_delete = evaluate_where_condition(where_cond, outs.data(), tpd);
    
    if (should_delete)
    {
      deleted_count++;
    }
    else
    {
      // Keep this record
      kept_records.push_back(std::vector<unsigned char>(rec.begin(), rec.end()));
    }
  }
  std::fclose(fp);
  
  // Rewrite table file with kept records
  fp = std::fopen(path.c_str(), "wb");
  if (!fp) return -7;
  
  // Update header
  hdr.num_records = (int)kept_records.size();
  hdr.file_size = hdr.record_offset + hdr.num_records * hdr.record_size;
  hdr.tpd_ptr = nullptr;  // Zero before writing
  
  size_t wr = std::fwrite(&hdr, 1, sizeof(table_file_header), fp);
  if (wr != sizeof(table_file_header)) { 
    std::fclose(fp); 
    return -8; 
  }
  
  // Write kept records
  for (size_t i = 0; i < kept_records.size(); ++i)
  {
    wr = std::fwrite(kept_records[i].data(), 1, hdr.record_size, fp);
    if (wr != (size_t)hdr.record_size) { 
      std::fclose(fp); 
      return -9; 
    }
  }
  
  std::fflush(fp);
  std::fclose(fp);
  
  // Print result
  if (deleted_count == 0)
  {
    printf("Warning: 0 rows deleted.\n");
  }
  else
  {
    printf("%d row(s) deleted.\n", deleted_count);
  }
  
  return 0;
}

// Semantic handler for DELETE statement
int sem_delete(token_list *t_list)
{
  // Expect: IDENT [WHERE ...] EOC
  if (!t_list) return INVALID_STATEMENT;
  
  token_list *cur = t_list;
  
  // 1) Table name
  if (!(cur->tok_class == identifier || cur->tok_class == keyword || cur->tok_class == type_name))
    return INVALID_TABLE_NAME;
  
  const char *table_name = cur->tok_string;
  
  // Get TPD to pass to WHERE parser
  tpd_entry *tpd = get_tpd_from_list((char*)table_name);
  if (!tpd) {
    initialize_tpd_list();
    tpd = get_tpd_from_list((char*)table_name);
    if (!tpd) return TABLE_NOT_EXIST;
  }
  
  cur = cur->next;
  if (!cur) return INVALID_STATEMENT;
  
  // 2) Parse optional WHERE clause
  where_condition *where_cond = NULL;
  
  if (cur->tok_value == K_WHERE)
  {
    cur = cur->next;
    where_cond = parse_where_clause(&cur, tpd);
    if (!where_cond)
    {
      return INVALID_STATEMENT;
    }
  }
  
  // 3) Expect EOC
  if (!cur || cur->tok_value != EOC)
  {
    free_where_conditions(where_cond);
    return INVALID_STATEMENT;
  }
  
  // 4) Execute delete
  int rc = delete_records(table_name, where_cond);
  
  free_where_conditions(where_cond);
  
  if (rc != 0)
  {
    printf("ERROR: delete failed (rc=%d)\n", rc);
    return rc;
  }
  
  return 0;
}

// Update records in table based on WHERE condition
int update_records(const char *table_name, const char *set_col, const char *set_value, where_condition *where_cond)
{
  if (!table_name || !set_col || !set_value) return -1;
  
  // Get TPD
  tpd_entry *tpd = get_tpd_from_list((char*)table_name);
  if (!tpd) {
    initialize_tpd_list();
    tpd = get_tpd_from_list((char*)table_name);
    if (!tpd) return TABLE_NOT_EXIST;
  }
  
  cd_entry *columns = (cd_entry *)((char *)tpd + tpd->cd_offset);
  
  // Find the column to update
  int set_col_index = -1;
  cd_entry *set_col_def = NULL;
  for (int i = 0; i < tpd->num_columns; i++)
  {
    if (strcasecmp(columns[i].col_name, set_col) == 0)
    {
      set_col_index = i;
      set_col_def = &columns[i];
      break;
    }
  }
  
  if (set_col_index == -1) return COLUMN_NOT_EXIST;
  
  // Validate NOT NULL constraint
  bool is_null_value = (strcmp(set_value, "NULL") == 0);
  if (is_null_value && set_col_def->not_null)
  {
    return INVALID_COLUMN_DEFINITION;
  }
  
  // Load table header
  table_file_header hdr;
  int rc = load_table_header(table_name, &hdr);
  if (rc != 0) {
    std::string path = std::string(table_name) + ".tab";
    FILE *fp = std::fopen(path.c_str(), "rb");
    if (!fp) return -2;
    size_t rd = std::fread(&hdr, 1, sizeof(table_file_header), fp);
    std::fclose(fp);
    if (rd != sizeof(table_file_header)) return -2;
  }
  hdr.tpd_ptr = tpd;
  
  // Open table file and read all records
  std::string path = table_filename(table_name);
  FILE *fp = std::fopen(path.c_str(), "rb");
  if (!fp) return -3;
  
  if (std::fseek(fp, hdr.record_offset, SEEK_SET) != 0) { 
    std::fclose(fp); 
    return -4; 
  }
  
  std::vector<unsigned char> rec(hdr.record_size);
  std::vector<std::string> bufs(tpd->num_columns);
  std::vector<char*> outs(tpd->num_columns);
  const int OUT_CAP = 512;
  for (int i = 0; i < tpd->num_columns; ++i) {
    bufs[i].assign(OUT_CAP, '\0');
    outs[i] = &bufs[i][0];
  }
  
  // Read all rows and update matching ones
  std::vector<std::vector<unsigned char> > all_records;
  int updated_count = 0;
  
  for (int r = 0; r < hdr.num_records; ++r)
  {
    size_t rd = std::fread(rec.data(), 1, hdr.record_size, fp);
    if (rd != (size_t)hdr.record_size) { 
      std::fclose(fp); 
      return -5; 
    }
    
    rc = unpack_row_to_strings(tpd, rec.data(), outs.data(), OUT_CAP);
    if (rc != 0) { 
      std::fclose(fp); 
      return -6; 
    }
    
    // Evaluate WHERE condition
    bool should_update = evaluate_where_condition(where_cond, outs.data(), tpd);
    
    if (should_update)
    {
      // Update the column value
      outs[set_col_index] = (char*)set_value;
      
      // Pack updated row
      std::vector<unsigned char> updated_rec(hdr.record_size);
      std::vector<const char*> vals(tpd->num_columns);
      for (int i = 0; i < tpd->num_columns; i++)
        vals[i] = outs[i];
      
      rc = pack_row_from_values(tpd, vals.data(), updated_rec.data(), hdr.record_size);
      if (rc != 0) { 
        std::fclose(fp); 
        return -7; 
      }
      
      all_records.push_back(updated_rec);
      updated_count++;
    }
    else
    {
      // Keep original record
      all_records.push_back(std::vector<unsigned char>(rec.begin(), rec.end()));
    }
  }
  std::fclose(fp);
  
  // Rewrite table file with updated records
  fp = std::fopen(path.c_str(), "wb");
  if (!fp) return -8;
  
  // Header stays the same except tpd_ptr
  hdr.tpd_ptr = nullptr;
  
  size_t wr = std::fwrite(&hdr, 1, sizeof(table_file_header), fp);
  if (wr != sizeof(table_file_header)) { 
    std::fclose(fp); 
    return -9; 
  }
  
  // Write all records
  for (size_t i = 0; i < all_records.size(); ++i)
  {
    wr = std::fwrite(all_records[i].data(), 1, hdr.record_size, fp);
    if (wr != (size_t)hdr.record_size) { 
      std::fclose(fp); 
      return -10; 
    }
  }
  
  std::fflush(fp);
  std::fclose(fp);
  
  // Print result
  if (updated_count == 0)
  {
    printf("Warning: 0 rows updated.\n");
  }
  else
  {
    printf("%d row(s) updated.\n", updated_count);
  }
  
  return 0;
}

// Semantic handler for UPDATE statement
int sem_update(token_list *t_list)
{
  // Expect: IDENT SET column = value [WHERE ...] EOC
  if (!t_list) return INVALID_STATEMENT;
  
  token_list *cur = t_list;
  
  // 1) Table name
  if (!(cur->tok_class == identifier || cur->tok_class == keyword || cur->tok_class == type_name))
    return INVALID_TABLE_NAME;
  
  const char *table_name = cur->tok_string;
  
  // Get TPD
  tpd_entry *tpd = get_tpd_from_list((char*)table_name);
  if (!tpd) {
    initialize_tpd_list();
    tpd = get_tpd_from_list((char*)table_name);
    if (!tpd) return TABLE_NOT_EXIST;
  }
  
  cur = cur->next;
  if (!cur || cur->tok_value != K_SET) return INVALID_STATEMENT;
  
  // 2) Parse SET clause
  cur = cur->next;
  if (!cur || !(cur->tok_class == identifier || cur->tok_class == keyword || cur->tok_class == type_name))
    return INVALID_COLUMN_NAME;
  
  const char *set_col = cur->tok_string;
  
  cur = cur->next;
  if (!cur || cur->tok_value != S_EQUAL) return INVALID_STATEMENT;
  
  cur = cur->next;
  if (!cur || (cur->tok_class != constant && cur->tok_value != K_NULL))
    return INVALID_STATEMENT;
  
  const char *set_value;
  char value_buf[MAX_TOK_LEN];
  if (cur->tok_value == K_NULL)
    strcpy(value_buf, "NULL");
  else
    strcpy(value_buf, cur->tok_string);
  set_value = value_buf;
  
  cur = cur->next;
  if (!cur) return INVALID_STATEMENT;
  
  // 3) Parse optional WHERE clause
  where_condition *where_cond = NULL;
  
  if (cur->tok_value == K_WHERE)
  {
    cur = cur->next;
    where_cond = parse_where_clause(&cur, tpd);
    if (!where_cond)
    {
      return INVALID_STATEMENT;
    }
  }
  
  // 4) Expect EOC
  if (!cur || cur->tok_value != EOC)
  {
    free_where_conditions(where_cond);
    return INVALID_STATEMENT;
  }
  
  // 5) Execute update
  int rc = update_records(table_name, set_col, set_value, where_cond);
  
  free_where_conditions(where_cond);
  
  if (rc != 0)
  {
    printf("ERROR: update failed (rc=%d)\n", rc);
    return rc;
  }
  
  return 0;
}

int sem_insert(token_list *t_list)
{
  // Expect: IDENT  K_VALUES  S_LEFT_PAREN  <val>{, <val>}  S_RIGHT_PAREN  EOC
  // Where <val> is STRING_LITERAL | INT_LITERAL | K_NULL

  if (!t_list) return INVALID_TABLE_DEFINITION;

  // 1) Table name
  token_list *cur = t_list;
  if (!(cur->tok_class == identifier || cur->tok_class == keyword || cur->tok_class == type_name))
    return INVALID_TABLE_NAME;

  const char *table_name = cur->tok_string;

  // 2) VALUES
  cur = cur->next;
  if (!cur || cur->tok_value != K_VALUES) return INVALID_STATEMENT;

  // 3) '('
  cur = cur->next;
  if (!cur || cur->tok_value != S_LEFT_PAREN) return INVALID_STATEMENT;

  // 4) Collect values
  std::vector<const char*> vals;
  cur = cur->next; // first token after '('

  for (;;)
  {
    if (!cur) return INVALID_STATEMENT;

    if (cur->tok_value == K_NULL)
    {
      vals.push_back("NULL");
    }
    else if (cur->tok_class == constant && cur->tok_value == STRING_LITERAL)
    {
      vals.push_back(cur->tok_string);   // de-quoted in tokenizer
    }
    else if (cur->tok_class == constant && cur->tok_value == INT_LITERAL)
    {
      vals.push_back(cur->tok_string);   // decimal text for atoi
    }
    else
    {
      return INVALID_STATEMENT;
    }

    cur = cur->next;
    if (!cur) return INVALID_STATEMENT;

    if (cur->tok_value == S_COMMA)
    {
      cur = cur->next;       // next literal
      continue;
    }
    else if (cur->tok_value == S_RIGHT_PAREN)
    {
      break;                 // done list
    }
    else
    {
      return INVALID_STATEMENT;
    }
  }

  // 5) EOC
  cur = cur->next;
  if (!cur || cur->tok_value != EOC) return INVALID_STATEMENT;

  // 6) Call helper to pack + append
  int rc = insert_values(table_name, vals.data(), (int)vals.size());
  if (rc != 0)
  {
    printf("ERROR: insert failed (rc=%d)\n", rc);
    return rc;
  }

  return 0;
}