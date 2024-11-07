

#include "hdKingAPI.h"

#include "error_code.h"

#include "data_types.h"

#include "hd3Struct.h"

#include <stdio.h>

#include <string.h>

int main()

{

    int32 nRet = RD_SUCCESS;

    HD3Connection conn;

    HD3PtTagProp tagSave;

    HD3PtTagProp tagQuery;

    const char *szTagName_1 = "szTagName_1";

    const char *szTagName_2 = "szTagName_2";

    HD3FilterItemSet queryItemset;

    HD3FilterItem filterItem[2];

    int64 nTagClassMask = 0;

    HD3PtTagProp HD3PtTagProps[10];

    int32 nTotalTagNum = 10;

    int32 nTagIDs[10];

    int32 nErrCode[10];

    HD3Mask mask;

    uint32 nPtTagID = 0;

    HD3HANDLE hIter = NULL;

    int32 nTagNum = 0;

    int32 i = 0;

    strcpy(conn.szAddress, "127.0.0.1");

    conn.nPort = 5673;

    conn.nTimeout = 3;

    nRet = nt3_connect(&conn);

    if (nRet != RD_SUCCESS)

    {

        printf("connect to server failed, error code[%d]!\n", nRet);

        return -1;
    }

    // login

    nRet = sc3_login("admin", "admin");

    if (nRet != RD_SUCCESS)

    {

        printf("login failed, error code[%d]!\n", nRet);

        return -1;
    }

    printf("login successful!\n");

    // add two tag

    strcpy(tagSave.szTagName, szTagName_1);

    tagSave.nTagType = HD3_TAG_TYPE_INT32;

    mask.nCommMask = HD3M_COMM_PROP_TAG_NAME | HD3M_COMM_PROP_TAG_TYPE;

    mask.nExtMask = 0;

    nRet = pt3_add_tag(&tagSave, &mask, "", &nPtTagID);

    if (nRet != RD_SUCCESS)

    {

        printf("add tags[%s] failed, error code [%d]!\n", tagSave.szTagName, nRet);

        nt3_disconnect();

        return -1;
    }

    printf("add tags[%s] successful!\n", tagSave.szTagName);

    strcpy(tagSave.szTagName, szTagName_2);

    tagSave.nTagType = HD3_TAG_TYPE_STRING;

    nRet = pt3_add_tag(&tagSave, &mask, "", &nPtTagID);

    if (nRet != RD_SUCCESS)

    {

        printf("add tags[%s] failed, error code [%d]!\n", tagSave.szTagName, nRet);

        nt3_disconnect();

        return -1;
    }

    printf("add tags[%s] successful!\n", tagSave.szTagName);

    // add ten tags

    uint32 *pTagIDs = NULL;

    pTagIDs = new uint32[10];

    memset((void *)pTagIDs, 0, sizeof(uint32) * 10);

    int32 *pErrCodes = NULL;

    pErrCodes = new int32[10];

    memset((void *)pErrCodes, 0, sizeof(int32) * 10);

    HD3Mask *pPropMasks;

    memset(HD3PtTagProps, 0, sizeof(HD3PtTagProp) * 10);

    pPropMasks = new HD3Mask[10];

    memset((void *)pPropMasks, 0, sizeof(HD3Mask) * 10);

    memset(nTagIDs, 0, sizeof(int32) * 10);

    memset(nErrCode, 0, sizeof(int32) * 10);

    // set HD3PtTagProp

    for (i = 0; i < nTotalTagNum; ++i)

    {

        HD3PtTagProps[i].nTagType = HD3_TAG_TYPE_INT32;

        sprintf(HD3PtTagProps[i].szTagName, "tagfix%d", i);

        pPropMasks[i].nCommMask = HD3M_COMM_PROP_TAG_NAME | HD3M_COMM_PROP_TAG_TYPE;

        pPropMasks[i].nExtMask = 0;
    }

    nRet = pt3_add_tags(nTotalTagNum, HD3PtTagProps, pPropMasks, "", pTagIDs, pErrCodes);

    delete[] pPropMasks;

    if (RD_SUCCESS != nRet)

    {

        printf("Failed to add tags!nRet = %d\n", nRet);

        nt3_disconnect();

        return -1;
    }

    printf("add tags successfully nRet = %d\n", nRet);

    // query tags by condition

    filterItem[0].nItemID = HD3_TAG_COL_COMM_PROP_TAG_NAME;

    filterItem[0].nRelation = HD3_RELATION_LIKE;

    strcpy(filterItem[0].szValue, "*");

    filterItem[1].nItemID = HD3_TAG_COL_COMM_PROP_TAG_TYPE;

    filterItem[1].nRelation = HD3_RELATION_EQUAL;

    sprintf(filterItem[1].szValue, "%d", HD3_TAG_TYPE_STRING);

    nTagClassMask = HD3M_ALL;

    queryItemset.nSize = 2;

    queryItemset.pItem = filterItem;

    nRet = pt3_query_tags_cond(&queryItemset, &mask, &hIter);

    if (nRet != RD_SUCCESS)

    {

        printf("query tags by condition failed!\n");

        nt3_disconnect();

        return -1;
    }

    nTagNum = 0;

    while (true)

    {

        nRet = ut3_get_item_step(hIter, &tagQuery);

        if (RD_SUCCESS == nRet)

        {

            nTagNum++;

            printf("query tag successful, tag name is %s, tag type is %d!\n", tagQuery.szTagName, tagQuery.nTagType);
        }

        else if (EC_HD_API_QUERY_END == nRet)

        {

            printf("query tag complete!");

            break;
        }

        else

        {

            printf("query tag failed, error code [%d]!", nRet);

            break;
        }
    }

    printf("you have queried %d tags totally!\n", nTagNum);

    ut3_free_handle(hIter);

    // query page tags by condition

    HD3PageQueryTagParam hdparam;

    hdparam.itemSet.pItem = queryItemset.pItem;

    hdparam.itemSet.nSize = queryItemset.nSize;

    hdparam.mask = mask;

    hdparam.nCapacity = 20;

    hdparam.nStartID = 0;

    nRet = pt3_query_specify_page_tags_cond(&hdparam, &hIter);

    if (nRet != RD_SUCCESS)

    {

        printf("pt_query_specify_page_tags_cond failed!\n");

        nt3_disconnect();

        return -1;
    }

    nTagNum = 0;

    while (true)

    {

        nRet = ut3_get_item_step(hIter, &tagQuery);

        if (RD_SUCCESS == nRet)

        {

            nTagNum++;

            printf("query tag successful, tag name is %s, tag type is %d!\n", tagQuery.szTagName, tagQuery.nTagType);
        }

        else if (EC_HD_API_QUERY_END == nRet)

        {

            printf("query tag complete!");

            break;
        }

        else

        {

            printf("query tag failed, error code [%d]!", nRet);

            break;
        }
    }

    printf("you have queried %d tags totally!\n", nTagNum);

    ut3_free_handle(hIter);

    // query tag id

    uint32 nTagID = 0;

    nRet = tag3_query_id_by_name(szTagName_1, &nTagID);

    if (nRet != RD_SUCCESS)

    {

        printf("query tag id failed, tag name[%s], error code[%d]!\n", szTagName_1, nRet);

        nt3_disconnect();

        return -1;
    }

    printf("tag[%s] query tag id successful, tag id[%d]!\n", szTagName_1, nTagID);

    // query tag type

    HD3_TAG_TYPE nTagType;

    nRet = pt3_query_tag_type(nTagID, &nTagType);

    if (nRet != RD_SUCCESS)

    {

        printf("query tag type failed, tag id[%d], error code[%d]!\n", nTagID, nRet);

        nt3_disconnect();

        return -1;
    }

    printf("query tag type successful, tag id[%d]!\n", nTagID);

    // modify tag desc

    HD3Mask hdMask;

    hdMask.nCommMask = HD3M_COMM_PROP_DESCRIPTOR;

    hdMask.nExtMask = 0;

    strcpy(tagSave.szDescriptor, "tag descriptor is modified");

    nRet = pt3_modify_tag_prop(nTagID, &tagSave, &hdMask);

    if (nRet != RD_SUCCESS)

    {

        printf("modify tag failed, tag id[%d], error code[%d]!\n", nTagID, nRet);

        nt3_disconnect();

        return -1;
    }

    printf("modify tag successful, tag id[%d]!\n", nTagID);

    // modify tag desc

    hdMask;

    hdMask.nCommMask = HD3M_COMM_PROP_DESCRIPTOR;

    hdMask.nExtMask = 0;

    strcpy(tagSave.szDescriptor, "tag descriptor is modified");

    nRet = pt3_modify_tags_prop(1, &nTagID, &tagSave, &hdMask, &nErrCode[0]);

    if (nRet != RD_SUCCESS)

    {

        printf("modify tag failed, tag id[%d], error code[%d]!\n", nTagID, nRet);

        nt3_disconnect();

        return -1;
    }

    printf("modify tag successful, tag id[%d]!\n", nTagID);

    // query tag prop

    nRet = pt3_query_tag_prop(nTagID, &tagQuery);

    if (nRet != RD_SUCCESS)

    {

        printf("query tag prop failed, tag id[%d], error code[%d]!\n", nTagID, nRet);

        nt3_disconnect();

        return -1;
    }

    printf("query tag prop successful, tag id[%d], tag name[%s], tag description[%s]!\n",

           tagQuery.nTagID, tagQuery.szTagName, tagQuery.szDescriptor);

    // delete tag

    nRet = tag3_delete_tag(nTagID);

    if (nRet != RD_SUCCESS)

    {

        printf("delete tag failed, tag id[%d], error code[%d]!\n", nTagID, nRet);

        nt3_disconnect();

        return -1;
    }

    printf("delete tag successful, tag id[%d]!\n", nTagID);

    nt3_disconnect();

    return 0;
}
