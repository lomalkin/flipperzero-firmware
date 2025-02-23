#include "record.h"
#include "check.h"
#include "memmgr.h"

#include <cmsis_os2.h>
#include <m-string.h>
#include <m-dict.h>

#define FURI_RECORD_FLAG_READY (0x1)

typedef struct {
    osEventFlagsId_t flags;
    void* data;
    size_t holders_count;
} FuriRecordData;

DICT_DEF2(FuriRecordDataDict, string_t, STRING_OPLIST, FuriRecordData, M_POD_OPLIST)

typedef struct {
    osMutexId_t mutex;
    FuriRecordDataDict_t records;
} FuriRecord;

static FuriRecord* furi_record = NULL;

void furi_record_init() {
    furi_record = furi_alloc(sizeof(FuriRecord));
    furi_record->mutex = osMutexNew(NULL);
    furi_check(furi_record->mutex);
    FuriRecordDataDict_init(furi_record->records);
}

static FuriRecordData* furi_record_data_get_or_create(string_t name_str) {
    furi_assert(furi_record);
    FuriRecordData* record_data = FuriRecordDataDict_get(furi_record->records, name_str);
    if(!record_data) {
        FuriRecordData new_record;
        new_record.flags = osEventFlagsNew(NULL);
        new_record.data = NULL;
        new_record.holders_count = 0;
        FuriRecordDataDict_set_at(furi_record->records, name_str, new_record);
        record_data = FuriRecordDataDict_get(furi_record->records, name_str);
    }
    return record_data;
}

static void furi_record_lock() {
    furi_check(osMutexAcquire(furi_record->mutex, osWaitForever) == osOK);
}

static void furi_record_unlock() {
    furi_check(osMutexRelease(furi_record->mutex) == osOK);
}

bool furi_record_exists(const char* name) {
    furi_assert(furi_record);
    furi_assert(name);

    bool ret = false;

    string_t name_str;
    string_init_set_str(name_str, name);

    furi_record_lock();
    ret = (FuriRecordDataDict_get(furi_record->records, name_str) != NULL);
    furi_record_unlock();

    string_clear(name_str);

    return ret;
}

void furi_record_create(const char* name, void* data) {
    furi_assert(furi_record);

    string_t name_str;
    string_init_set_str(name_str, name);

    furi_record_lock();

    // Get record data and fill it
    FuriRecordData* record_data = furi_record_data_get_or_create(name_str);
    furi_assert(record_data->data == NULL);
    record_data->data = data;
    osEventFlagsSet(record_data->flags, FURI_RECORD_FLAG_READY);

    furi_record_unlock();

    string_clear(name_str);
}

bool furi_record_destroy(const char* name) {
    furi_assert(furi_record);

    bool ret = false;

    string_t name_str;
    string_init_set_str(name_str, name);

    furi_record_lock();

    FuriRecordData* record_data = FuriRecordDataDict_get(furi_record->records, name_str);
    furi_assert(record_data);
    if(record_data->holders_count == 0) {
        furi_check(osOK == osEventFlagsDelete(record_data->flags));
        FuriRecordDataDict_erase(furi_record->records, name_str);
        ret = true;
    }

    furi_record_unlock();

    string_clear(name_str);

    return ret;
}

void* furi_record_open(const char* name) {
    furi_assert(furi_record);

    string_t name_str;
    string_init_set_str(name_str, name);

    furi_record_lock();

    FuriRecordData* record_data = furi_record_data_get_or_create(name_str);
    record_data->holders_count++;

    furi_record_unlock();

    // Wait for record to become ready
    furi_check(
        osEventFlagsWait(
            record_data->flags,
            FURI_RECORD_FLAG_READY,
            osFlagsWaitAny | osFlagsNoClear,
            osWaitForever) == FURI_RECORD_FLAG_READY);

    string_clear(name_str);

    return record_data->data;
}

void furi_record_close(const char* name) {
    furi_assert(furi_record);

    string_t name_str;
    string_init_set_str(name_str, name);

    furi_record_lock();

    FuriRecordData* record_data = FuriRecordDataDict_get(furi_record->records, name_str);
    furi_assert(record_data);
    record_data->holders_count--;

    furi_record_unlock();

    string_clear(name_str);
}
