#include "hdb_locl.h"

RCSID("$Id$");

#ifdef HAVE_NDBM_H

krb5_error_code
NDBM_close(krb5_context context, HDB *db)
{
    DBM *d = (DBM*)db->db;
    dbm_close(d);
    free(db);
    return 0;
}

static krb5_error_code
NDBM_fetch(krb5_context context, HDB *db, hdb_entry *entry)
{
    DBM *d = (DBM*)db->db;
    datum key, value;
    krb5_data data;
    int err;

    hdb_principal2key(context, entry->principal, &data);

    key.dptr = data.data;
    key.dsize = data.length;
    value = dbm_fetch(d, key);
    krb5_data_free(&data);
    if(value.dptr == NULL)
	return errno;
    
    data.data = value.dptr;
    data.length = value.dsize;
    
    hdb_value2entry(context, &data, entry);
    krb5_data_free(&data);
    return 0;
}

static krb5_error_code
NDBM_store(krb5_context context, HDB *db, hdb_entry *entry)
{
    DBM *d = (DBM*)db->db;
    krb5_data data;
    int err;
    datum key, value;
    hdb_principal2key(context, entry->principal, &data);
    key.dptr = data.data;
    key.dsize = data.length;
    hdb_entry2value(context, entry, &data);
    value.dptr = data.data;
    value.dsize = data.length;
    err = dbm_store(d, key, value, DBM_REPLACE);
    free(key.dptr);
    free(value.dptr);
    if(err < 0)
	return errno;
    return 0;
}

static krb5_error_code
NDBM_delete(krb5_context context, HDB *db, hdb_entry *entry)
{
    DBM *d = (DBM*)db->db;
    datum key;
    krb5_data data;
    int err;

    hdb_principal2key(context, entry->principal, &data);

    key.dptr = data.data;
    key.dsize = data.length;
    err = dbm_delete(d, key);
    krb5_data_free(&data);
    if(err < 0)
	return errno;
    return 0;
}

static krb5_error_code
NDBM_seq(krb5_context context, HDB *db, hdb_entry *entry, int first)

{
    DBM *d = (DBM*)db->db;
    datum key, value;
    krb5_data data;

    if(first)
	key = dbm_firstkey(d);
    else
	key = dbm_nextkey(d);
    if(key.dptr == NULL)
	return KRB5_HDB_NOENTRY;
    data.data = key.dptr;
    data.length = key.dsize;
    hdb_key2principal(context, &data, &entry->principal);
    value = dbm_fetch(d, key);
    krb5_data_free(&data);
    data.data = value.dptr;
    data.length = value.dsize;
    hdb_value2entry(context, &data, entry);
    krb5_data_free(&data);
    return 0;
}


static krb5_error_code
NDBM_firstkey(krb5_context context, HDB *db, hdb_entry *entry)
{
    return NDBM_seq(context, db, entry, 1);
}


static krb5_error_code
NDBM_nextkey(krb5_context context, HDB *db, hdb_entry *entry)
{
    return NDBM_seq(context, db, entry, 0);
}

krb5_error_code
hdb_ndbm_open(krb5_context context, HDB **db, 
	      const char *filename, int flags, mode_t mode)
{
    DBM *d;
    char *fn = malloc(strlen(filename) + 4);
    sprintf(fn, "%s.db", filename);
    d = dbm_open(fn, flags, mode);
    free(fn);
    if(d == NULL)
	return errno;
    *db = malloc(sizeof(**db));
    (*db)->db = d;
    (*db)->close = NDBM_close;
    (*db)->fetch = NDBM_fetch;
    (*db)->store = NDBM_store;
    (*db)->delete = NDBM_delete;
    (*db)->firstkey = NDBM_firstkey;
    (*db)->nextkey= NDBM_nextkey;
    return 0;
}

#endif
