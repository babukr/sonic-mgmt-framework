////////////////////////////////////////////////////////////////////////////////
//                                                                            //
//  Copyright 2019 Dell, Inc.                                                 //
//                                                                            //
//  Licensed under the Apache License, Version 2.0 (the "License");           //
//  you may not use this file except in compliance with the License.          //
//  You may obtain a copy of the License at                                   //
//                                                                            //
//  http://www.apache.org/licenses/LICENSE-2.0                                //
//                                                                            //
//  Unless required by applicable law or agreed to in writing, software       //
//  distributed under the License is distributed on an "AS IS" BASIS,         //
//  WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.  //
//  See the License for the specific language governing permissions and       //
//  limitations under the License.                                            //
//                                                                            //
////////////////////////////////////////////////////////////////////////////////

package transformer

import (
    "fmt"
    "translib/db"
    "strings"
    "encoding/json"
    "strconv"
    "errors"
	"sync"
    "github.com/openconfig/goyang/pkg/yang"
    "github.com/openconfig/ygot/ygot"

    log "github.com/golang/glog"
)

type typeMapOfInterface map[string]interface{}
type typeChMapErr struct {
    result typeMapOfInterface
    err    error
}

var mapCopyMutex = &sync.Mutex{}

func xfmrHandlerFunc(inParams XfmrParams) (error) {
    xpath, _ := XfmrRemoveXPATHPredicates(inParams.uri)
    xfmrLogInfoAll("Subtree transformer function(\"%v\") invoked for yang path(\"%v\").", xYangSpecMap[xpath].xfmrFunc, xpath)
    _, err := XlateFuncCall(dbToYangXfmrFunc(xYangSpecMap[xpath].xfmrFunc), inParams)
    if err != nil {
        xfmrLogInfoAll("Failed to retrieve data for xpath(\"%v\") err(%v).", inParams.uri, err)
        return err
    }

    return err
}

func leafXfmrHandlerFunc(inParams XfmrParams) (map[string]interface{}, error) {
    xpath, _ := XfmrRemoveXPATHPredicates(inParams.uri)
    ret, err := XlateFuncCall(dbToYangXfmrFunc(xYangSpecMap[xpath].xfmrField), inParams)
    if err != nil {
        return nil, err
    }
    if ret != nil {
        fldValMap := ret[0].Interface().(map[string]interface{})
        return fldValMap, nil
    } else {
        return nil, nil
    }
}

func validateHandlerFunc(inParams XfmrParams) (bool) {
    xpath, _ := XfmrRemoveXPATHPredicates(inParams.uri)
    ret, err := XlateFuncCall(xYangSpecMap[xpath].validateFunc, inParams)
    if err != nil {
        return false
    }
    return ret[0].Interface().(bool)
}

func xfmrTblHandlerFunc(xfmrTblFunc string, inParams XfmrParams) ([]string, error) {
	xfmrLogInfoAll("Received inParams %v, table transformer function name %v", inParams, xfmrTblFunc)
	var retTblLst []string
	ret, err := XlateFuncCall(xfmrTblFunc, inParams)
	if err != nil {
		return retTblLst, err
        }
	if ((ret != nil) && (len(ret)>0)) {
		if len(ret) == TBL_XFMR_RET_ARGS {
			// table xfmr returns err as second value in return data list from <xfmr_func>.Call()
			 if ret[TBL_XFMR_RET_ERR_INDX].Interface() != nil {
				 err = ret[TBL_XFMR_RET_ERR_INDX].Interface().(error)
				 if err != nil {
					 log.Warningf("Transformer function(\"%v\") returned error - %v.", xfmrTblFunc, err)
					 return retTblLst, err
				 }
			 }
		}

		if ret[TBL_XFMR_RET_VAL_INDX].Interface() != nil {
			retTblLst = ret[TBL_XFMR_RET_VAL_INDX].Interface().([]string)
		}
	}
	return retTblLst, err
}


func DbValToInt(dbFldVal string, base int, size int, isUint bool) (interface{}, error) {
	var res interface{}
	var err error
	if isUint {
		if res, err = strconv.ParseUint(dbFldVal, base, size); err != nil {
			log.Warningf("Non Yint%v type for yang leaf-list item %v", size, dbFldVal)
		}
	} else {
		if res, err = strconv.ParseInt(dbFldVal, base, size); err != nil {
			log.Warningf("Non Yint %v type for yang leaf-list item %v", size, dbFldVal)
		}
	}
	return res, err
}

func getLeafrefRefdYangType(yngTerminalNdDtType yang.TypeKind, fldXpath string) (yang.TypeKind) {
	if yngTerminalNdDtType == yang.Yleafref {
		var entry *yang.Entry
		var path string
		if _, ok := xDbSpecMap[fldXpath]; ok {
			path = xDbSpecMap[fldXpath].dbEntry.Type.Path
			entry = xDbSpecMap[fldXpath].dbEntry
		} else if _, ok := xYangSpecMap[fldXpath]; ok {
			path = xYangSpecMap[fldXpath].yangEntry.Type.Path
			entry = xYangSpecMap[fldXpath].yangEntry
		}
		path = stripAugmentedModuleNames(path)
		path = path[1:]
		xfmrLogInfoAll("Received path %v for FieldXpath %v", path, fldXpath)
		if strings.Contains(path, "..") {
			if entry != nil && len(path) > 0 {
				// Referenced path within same yang file
				pathList := strings.Split(path, "/")
				for _, x := range pathList {
					if x == ".." {
						entry = entry.Parent
					} else {
						if _,ok := entry.Dir[x]; ok {
							entry = entry.Dir[x]
						}
					}
				}
				if entry != nil {
					yngTerminalNdDtType = entry.Type.Kind
					xfmrLogInfoAll("yangLeaf datatype %v", yngTerminalNdDtType)
					if yngTerminalNdDtType == yang.Yleafref {
						leafPath := getXpathFromYangEntry(entry)
						xfmrLogInfoAll("getLeafrefRefdYangType: xpath for leafref type:%v",leafPath)
						return getLeafrefRefdYangType(yngTerminalNdDtType, leafPath)
					}
				}
			}
		} else if len(path) > 0 {
			// Referenced path in a different yang file
			xpath, err := XfmrRemoveXPATHPredicates(path)
			if  err != nil {
				log.Warningf("error in XfmrRemoveXPATHPredicates %v", xpath)
				return yngTerminalNdDtType
			}
			// Form xpath based on sonic or non sonic yang path
			if strings.Contains(xpath, "sonic") {
				pathList := strings.Split(xpath, "/")
				xpath = pathList[SONIC_TABLE_INDEX]+ "/" + pathList[SONIC_FIELD_INDEX]
				if _, ok := xDbSpecMap[xpath]; ok {
					yngTerminalNdDtType = xDbSpecMap[xpath].dbEntry.Type.Kind
				}

			} else {
				xpath = replacePrefixWithModuleName(xpath)
				if _, ok := xYangSpecMap[xpath]; ok {
					yngTerminalNdDtType = xYangSpecMap[xpath].dbEntry.Type.Kind
				}
			}

		}
		xfmrLogInfoAll("yangLeaf datatype %v", yngTerminalNdDtType)
	}
	return yngTerminalNdDtType
}

func DbToYangType(yngTerminalNdDtType yang.TypeKind, fldXpath string, dbFldVal string) (interface{}, interface{}, error) {
	xfmrLogInfoAll("Received FieldXpath %v, yngTerminalNdDtType %v and Db field value %v to be converted to yang data-type.", fldXpath, yngTerminalNdDtType, dbFldVal)
	var res interface{}
	var resPtr interface{}
	var err error
	const INTBASE = 10

	if yngTerminalNdDtType == yang.Yleafref {
		yngTerminalNdDtType = getLeafrefRefdYangType(yngTerminalNdDtType, fldXpath)
	}

	switch yngTerminalNdDtType {
        case yang.Ynone:
                log.Warning("Yang node data-type is non base yang type")
		//TODO - enhance to handle non base data types depending on future use case
		err = errors.New("Yang node data-type is non base yang type")
        case yang.Yint8:
                res, err = DbValToInt(dbFldVal, INTBASE, 8, false)
		var resInt8 int8 = int8(res.(int64))
		resPtr = &resInt8
        case yang.Yint16:
                res, err = DbValToInt(dbFldVal, INTBASE, 16, false)
		var resInt16 int16 = int16(res.(int64))
		resPtr = &resInt16
        case yang.Yint32:
                res, err = DbValToInt(dbFldVal, INTBASE, 32, false)
		var resInt32 int32 = int32(res.(int64))
		resPtr = &resInt32
        case yang.Yuint8:
                res, err = DbValToInt(dbFldVal, INTBASE, 8, true)
		var resUint8 uint8 = uint8(res.(uint64))
		resPtr = &resUint8
        case yang.Yuint16:
                res, err = DbValToInt(dbFldVal, INTBASE, 16, true)
		var resUint16 uint16 = uint16(res.(uint64))
		resPtr = &resUint16
        case yang.Yuint32:
                res, err = DbValToInt(dbFldVal, INTBASE, 32, true)
		var resUint32 uint32 = uint32(res.(uint64))
		resPtr = &resUint32
        case yang.Ybool:
		if res, err = strconv.ParseBool(dbFldVal); err != nil {
			log.Warningf("Non Bool type for yang leaf-list item %v", dbFldVal)
		}
		var resBool bool = res.(bool)
		resPtr = &resBool
        case yang.Ybinary, yang.Ydecimal64, yang.Yenum, yang.Yidentityref, yang.Yint64, yang.Yuint64, yang.Ystring, yang.Yunion, yang.Yleafref:
                // TODO - handle the union type
                // Make sure to encode as string, expected by util_types.go: ytypes.yangToJSONType
                log.Info("Yenum/Ystring/Yunion(having all members as strings) type for yangXpath ", fldXpath)
                res = dbFldVal
		var resString string = res.(string)
		resPtr = &resString
	case yang.Yempty:
		logStr := fmt.Sprintf("Yang data type for xpath %v is Yempty.", fldXpath)
		log.Error(logStr)
		err = errors.New(logStr)
        default:
		logStr := fmt.Sprintf("Unrecognized/Unhandled yang-data type(%v) for xpath %v.", fldXpath, yang.TypeKindToName[yngTerminalNdDtType])
                log.Error(logStr)
                err = errors.New(logStr)
        }
	return res, resPtr, err
}

/*convert leaf-list in Db to leaf-list in yang*/
func processLfLstDbToYang(fieldXpath string, dbFldVal string, yngTerminalNdDtType yang.TypeKind) []interface{} {
	valLst := strings.Split(dbFldVal, ",")
	var resLst []interface{}
	const INTBASE = 10

	log.Info("xpath:", fieldXpath, ", dbFldVal:", dbFldVal)
	switch  yngTerminalNdDtType {
	case yang.Ybinary, yang.Ydecimal64, yang.Yenum, yang.Yidentityref, yang.Yint64, yang.Yuint64, yang.Ystring, yang.Yunion:
                // TODO - handle the union type.OC yang should have field xfmr.sonic-yang?
                // Make sure to encode as string, expected by util_types.go: ytypes.yangToJSONType:
		log.Info("DB leaf-list and Yang leaf-list are of same data-type")
		for _, fldVal := range valLst {
			resLst = append(resLst, fldVal)
		}
	default:
		for _, fldVal := range valLst {
			resVal, _, err := DbToYangType(yngTerminalNdDtType, fieldXpath, fldVal)
			if err == nil {
				resLst = append(resLst, resVal)
			}
		}
	}
	return resLst
}

func sonicDbToYangTerminalNodeFill(tblName string, field string, value string, resultMap map[string]interface{}) {
	resField := field
	if len(value) == 0 {
		return
	}
	if strings.HasSuffix(field, "@") {
		fldVals := strings.Split(field, "@")
		resField = fldVals[0]
	}
	fieldXpath := tblName + "/" + resField
	xDbSpecMapEntry, ok := xDbSpecMap[fieldXpath]
	if !ok {
		log.Warningf("No entry found in xDbSpecMap for xpath %v", fieldXpath)
		return
	}
	if xDbSpecMapEntry.dbEntry == nil {
		log.Warningf("Yang entry is nil in xDbSpecMap for xpath %v", fieldXpath)
		return
	}

	yangType := yangTypeGet(xDbSpecMapEntry.dbEntry)
	yngTerminalNdDtType := xDbSpecMapEntry.dbEntry.Type.Kind
	if yangType ==  YANG_LEAF_LIST {
		/* this should never happen but just adding for safetty */
		if !strings.HasSuffix(field, "@") {
			log.Warningf("Leaf-list in Sonic yang should also be a leaf-list in DB, its not for xpath %v", fieldXpath)
			return
		}
		resLst := processLfLstDbToYang(fieldXpath, value, yngTerminalNdDtType)
		resultMap[resField] = resLst
	} else { /* yangType is leaf - there are only 2 types of yang terminal node leaf and leaf-list */
		resVal, _, err := DbToYangType(yngTerminalNdDtType, fieldXpath, value)
		if err != nil {
			log.Warningf("Failure in converting Db value type to yang type for xpath", fieldXpath)
		} else {
			resultMap[resField] = resVal
		}
	}
	return
}

func sonicDbToYangListFill(uri string, xpath string, dbIdx db.DBNum, table string, key string, dbDataMap *map[db.DBNum]map[string]map[string]db.Value) []typeMapOfInterface {
	var mapSlice []typeMapOfInterface
	dbTblData := (*dbDataMap)[dbIdx][table]

	for keyStr, _ := range dbTblData {
		curMap := make(map[string]interface{})
		sonicDbToYangDataFill(uri, xpath, dbIdx, table, keyStr, dbDataMap, curMap)
		dbSpecData, ok := xDbSpecMap[table]
		if ok && dbSpecData.keyName == nil {
			yangKeys := yangKeyFromEntryGet(xDbSpecMap[xpath].dbEntry)
			sonicKeyDataAdd(dbIdx, yangKeys, table, keyStr, curMap)
		}
		if curMap != nil && len(curMap) > 0 {
			mapSlice = append(mapSlice, curMap)
		}
	}
	return mapSlice
}

func sonicDbToYangDataFill(uri string, xpath string, dbIdx db.DBNum, table string, key string, dbDataMap *map[db.DBNum]map[string]map[string]db.Value, resultMap map[string]interface{}) {
	yangNode, ok := xDbSpecMap[xpath]

	if ok  && yangNode.dbEntry != nil {
		xpathPrefix := table
		if len(table) > 0 { xpathPrefix += "/" }

		for yangChldName := range yangNode.dbEntry.Dir {
			chldXpath := xpathPrefix+yangChldName
			if xDbSpecMap[chldXpath] != nil && xDbSpecMap[chldXpath].dbEntry != nil {
				chldYangType := yangTypeGet(xDbSpecMap[chldXpath].dbEntry)

				if  chldYangType == YANG_LEAF || chldYangType == YANG_LEAF_LIST {
					xfmrLogInfoAll("tbl(%v), k(%v), yc(%v)", table, key, yangChldName)
					fldName := yangChldName
					if chldYangType == YANG_LEAF_LIST  {
						fldName = fldName + "@"
					}
					sonicDbToYangTerminalNodeFill(table, fldName, (*dbDataMap)[dbIdx][table][key].Field[fldName], resultMap)
				} else if chldYangType == YANG_CONTAINER {
					curMap := make(map[string]interface{})
					curUri := xpath + "/" + yangChldName
					// container can have a static key, so extract key for current container
					_, curKey, curTable := sonicXpathKeyExtract(curUri)
					// use table-name as xpath from now on
					sonicDbToYangDataFill(curUri, curTable, xDbSpecMap[curTable].dbIndex, curTable, curKey, dbDataMap, curMap)
					if len(curMap) > 0 {
						resultMap[yangChldName] = curMap
					} else {
						xfmrLogInfoAll("Empty container for xpath(%v)", curUri)
					}
				} else if chldYangType == YANG_LIST {
					pathList := strings.Split(uri, "/")
					// Skip the list entries if the uri has specific list query
					if len(pathList) > SONIC_TABLE_INDEX+1 && !strings.Contains(uri,yangChldName) {
						xfmrLogInfoAll("Skipping yangChldName: %v, pathList:%v, len:%v", yangChldName, pathList, len(pathList))
					} else {
						var mapSlice []typeMapOfInterface
						curUri := xpath + "/" + yangChldName
						mapSlice = sonicDbToYangListFill(curUri, curUri, dbIdx, table, key, dbDataMap)
						if len(key) > 0 && len(mapSlice) == 1 {// Single instance query. Don't return array of maps
							for k, val := range mapSlice[0] {
								resultMap[k] = val
							}

						} else if len(mapSlice) > 0 {
							resultMap[yangChldName] = mapSlice
						} else {
							xfmrLogInfoAll("Empty list for xpath(%v)", curUri)
						}
					}
				} else if chldYangType == YANG_CHOICE || chldYangType == YANG_CASE {
					curUri := table + "/" + yangChldName
					sonicDbToYangDataFill(curUri, curUri, xDbSpecMap[table].dbIndex, table, key, dbDataMap, resultMap)
				} else {
					xfmrLogInfoAll("Not handled case %v", chldXpath)
				}
			} else {
				xfmrLogInfoAll("Yang entry not found for %v", chldXpath)
			}
		}
	}
	return
}

/* Traverse db map and create json for cvl yang */
func directDbToYangJsonCreate(uri string, dbDataMap *map[db.DBNum]map[string]map[string]db.Value, resultMap map[string]interface{}) (string, error, bool) {
	var err error
	xpath, key, table := sonicXpathKeyExtract(uri)

	if len(xpath) > 0 {
		var dbNode *dbInfo

		if len(table) > 0 {
			tokens:= strings.Split(xpath, "/")
			if tokens[SONIC_TABLE_INDEX] == table {
				fieldName := tokens[len(tokens)-1]
				dbSpecField := table + "/" + fieldName
				_, ok := xDbSpecMap[dbSpecField]
				if ok && (xDbSpecMap[dbSpecField].fieldType == YANG_LEAF || xDbSpecMap[dbSpecField].fieldType == YANG_LEAF_LIST) {
					dbNode = xDbSpecMap[dbSpecField]
					xpath = dbSpecField
				} else {
					dbNode = xDbSpecMap[table]
				}
			}
		} else {
			dbNode, _ = xDbSpecMap[xpath]
		}

		if dbNode != nil && dbNode.dbEntry != nil {
			cdb   := db.ConfigDB
			yangType := yangTypeGet(dbNode.dbEntry)
			if len(table) > 0 {
				cdb = xDbSpecMap[table].dbIndex
			}

			if yangType == YANG_LEAF || yangType == YANG_LEAF_LIST {
				fldName := xDbSpecMap[xpath].dbEntry.Name
				if yangType == YANG_LEAF_LIST  {
					fldName = fldName + "@"
				}
				sonicDbToYangTerminalNodeFill(table, fldName, (*dbDataMap)[cdb][table][key].Field[fldName], resultMap)
			} else if yangType == YANG_CONTAINER {
				if len(table) > 0 {
					xpath = table
				}
				sonicDbToYangDataFill(uri, xpath, cdb, table, key, dbDataMap, resultMap)
			} else if yangType == YANG_LIST {
				mapSlice := sonicDbToYangListFill(uri, xpath, cdb, table, key, dbDataMap)
				if len(key) > 0 && len(mapSlice) == 1 {// Single instance query. Don't return array of maps
                                                for k, val := range mapSlice[0] {
                                                        resultMap[k] = val
                                                }

                                } else if len(mapSlice) > 0 {
					pathl := strings.Split(xpath, "/")
					lname := pathl[len(pathl) - 1]
					resultMap[lname] = mapSlice
				}
			}
		}
	}

	jsonMapData, _ := json.Marshal(resultMap)
	isEmptyPayload := isJsonDataEmpty(string(jsonMapData))
	jsonData := fmt.Sprintf("%v", string(jsonMapData))
        if isEmptyPayload {
		errStr := fmt.Sprintf("No data available")
		log.Error(errStr)
		//err = tlerr.NotFound("Resource not found")
        }
        return jsonData, err, isEmptyPayload
}

func tableNameAndKeyFromDbMapGet(dbDataMap map[string]map[string]db.Value) (string, string, error) {
    tableName := ""
    tableKey  := ""
    for tn, tblData := range dbDataMap {
        tableName = tn
        for kname, _ := range tblData {
            tableKey = kname
        }
    }
    return tableName, tableKey, nil
}

func fillDbDataMapForTbl(uri string, xpath string, tblName string, tblKey string, cdb db.DBNum, dbs [db.MaxDB]*db.DB) (map[db.DBNum]map[string]map[string]db.Value, error) {
	var err error
	dbresult  := make(RedisDbMap)
	dbresult[cdb] = make(map[string]map[string]db.Value)
	dbFormat := KeySpec{}
	dbFormat.Ts.Name = tblName
	dbFormat.dbNum = cdb
	if tblKey != "" {
		dbFormat.Key.Comp = append(dbFormat.Key.Comp, tblKey)
	}
	err = TraverseDb(dbs, dbFormat, &dbresult, nil)
	if err != nil {
		log.Errorf("TraverseDb() failure for tbl(DB num) %v(%v) for xpath %v", tblName, cdb, xpath)
		return nil, err
	}
	if _, ok := dbresult[cdb]; !ok {
		logStr := fmt.Sprintf("TraverseDb() did not populate Db data for tbl(DB num) %v(%v) for xpath %v", tblName, cdb, xpath)
		err = fmt.Errorf("%v", logStr)
		return nil, err
	}
	return dbresult, err

}

// Assumption: All tables are from the same DB
func dbDataFromTblXfmrGet(tbl string, inParams XfmrParams, dbDataMap *map[db.DBNum]map[string]map[string]db.Value) error {
    // skip the query if the table is already visited
    if _,ok := (*dbDataMap)[inParams.curDb][tbl]; ok {
       if len(inParams.key) > 0 {
          if  _,ok = (*dbDataMap)[inParams.curDb][tbl][inParams.key]; ok {
             return nil
          }
       } else {
          return nil
       }
    }
    xpath, _ := XfmrRemoveXPATHPredicates(inParams.uri)
    curDbDataMap, err := fillDbDataMapForTbl(inParams.uri, xpath, tbl, inParams.key, inParams.curDb, inParams.dbs)
    if err == nil {
        mapCopy((*dbDataMap)[inParams.curDb], curDbDataMap[inParams.curDb])
    }
    return nil
}

func yangListDataFill(dbs [db.MaxDB]*db.DB, ygRoot *ygot.GoStruct, uri string, requestUri string, xpath string, dbDataMap *map[db.DBNum]map[string]map[string]db.Value, resultMap map[string]interface{}, tbl string, tblKey string, cdb db.DBNum, validate bool, txCache interface{}, isFirstCall bool) error {
	var tblList []string

	_, ok := xYangSpecMap[xpath]
	if ok {
	if xYangSpecMap[xpath].xfmrTbl != nil {
		xfmrTblFunc := *xYangSpecMap[xpath].xfmrTbl
		if len(xfmrTblFunc) > 0 {
			inParams := formXfmrInputRequest(dbs[cdb], dbs, cdb, ygRoot, uri, requestUri, GET, tblKey, dbDataMap, nil, nil, txCache)
			tblList, _   = xfmrTblHandlerFunc(xfmrTblFunc, inParams)
			if len(tblList) != 0 {
				for _, curTbl := range tblList {
					dbDataFromTblXfmrGet(curTbl, inParams, dbDataMap)
				}
			}
		}
		if tbl != "" {
			tblList = append(tblList, tbl)
		}
	} else if tbl != "" && xYangSpecMap[xpath].xfmrTbl == nil {
		tblList = append(tblList, tbl)
	} else if tbl == "" && xYangSpecMap[xpath].xfmrTbl == nil {
		// Handling for case: Parent list is not associated with a tableName but has children containers/lists having tableNames.
		if tblKey != "" {
			var wg sync.WaitGroup
			var mapSlice []typeMapOfInterface
			wg.Add(1)
			chl := make(chan typeChMapErr)
			go yangListInstanceDataFill(dbs, ygRoot, uri, requestUri, xpath, dbDataMap, resultMap, tbl, tblKey, cdb, validate, txCache, isFirstCall, &wg, chl)
			wg.Wait()
			close(chl)
			for chData := range chl {
				if chData.err != nil {
					xfmrLogInfoAll("Error(%v) returned for %v", chData.err, uri)
				} else {
					if len(chData.result) > 0 {
						mapSlice = append(mapSlice, chData.result)
					} else {
						xfmrLogInfoAll("Empty map returned for (%v)", uri)
					}
				}
			}

			if len(mapSlice) > 0 {
				listInstanceGet := false
				// Check if it is a list instance level Get
				if ((strings.HasSuffix(uri, "]")) || (strings.HasSuffix(uri, "]/"))) {
					listInstanceGet = true
					for k, v := range mapSlice[0] {
						resultMap[k] = v
					}
				}
				if !listInstanceGet {
					resultMap[xYangSpecMap[xpath].yangEntry.Name] = mapSlice
				}
			}
		}
	}
	}

	var tblWg sync.WaitGroup
	for _, tbl = range(tblList) {
		tblWg.Add(1)

		defer func() {
                        if rc := recover(); rc != nil {
                                log.Errorf("Recover Table handling for :%v", tbl)
                        }
                }()

		go func(tbl string) {
		defer tblWg.Done()
		tblData, ok := (*dbDataMap)[cdb][tbl]

		if ok {
			var wg sync.WaitGroup
			var mapSlice []typeMapOfInterface
			chl := make(chan typeChMapErr, len(tblData))
			for dbKey, _ := range tblData {
				wg.Add(1)
				go yangListInstanceDataFill(dbs, ygRoot, uri, requestUri, xpath, dbDataMap, resultMap, tbl, dbKey, cdb, validate, txCache, isFirstCall, &wg, chl)
			}
			wg.Wait()
			close(chl)
			for chData := range chl {
				if chData.err != nil {
					xfmrLogInfoAll("Error(%v) returned for %v", chData.err, uri)
				} else {
					if len(chData.result) > 0 {
						mapSlice = append(mapSlice, chData.result)
					} else {
						xfmrLogInfoAll("Empty map returned for (%v)", uri)
					}
				}
			}

			if len(mapSlice) > 0 {
				listInstanceGet := false
				/*Check if it is a list instance level Get*/
				if ((strings.HasSuffix(uri, "]")) || (strings.HasSuffix(uri, "]/"))) {
					listInstanceGet = true
					for k, v := range mapSlice[0] {
						resultMap[k] = v
					}
				}
				if !listInstanceGet {
					if _, specOk := xYangSpecMap[xpath]; specOk {
					if _, ok := resultMap[xYangSpecMap[xpath].yangEntry.Name]; ok {
						mlen := len(resultMap[xYangSpecMap[xpath].yangEntry.Name].([]typeMapOfInterface))
						for i := 0; i < mlen; i++ {
							mapSlice = append(mapSlice, resultMap[xYangSpecMap[xpath].yangEntry.Name].([]typeMapOfInterface)[i])
						}
					}
					resultMap[xYangSpecMap[xpath].yangEntry.Name] = mapSlice
					}
				}
			} else {
				xfmrLogInfoAll("Empty slice for (\"%v\").\r\n", uri)
			}
		}
		}(tbl)
	}// end of tblList for
	tblWg.Wait()

	return nil
}

func yangListInstanceDataFill(dbs [db.MaxDB]*db.DB, ygRoot *ygot.GoStruct, uri string, requestUri string, xpath string, dbDataMap *map[db.DBNum]map[string]map[string]db.Value, resultMap map[string]interface{}, tbl string, dbKey string, cdb db.DBNum, validate bool, txCache interface{}, isFirstCall bool, wg *sync.WaitGroup, chl chan typeChMapErr) {

	var err error
	var chData typeChMapErr
	curMap := make(map[string]interface{})
	err = nil

	defer func() {
		if rc := recover(); rc != nil {
			log.Errorf("Recover List Instance handling for dbKey:%v", dbKey)
			chData.err    = err
			chData.result = curMap
			chl <- chData
			wg.Done()
		}
	}()

	curKeyMap, curUri, _ := dbKeyToYangDataConvert(uri, requestUri, xpath, dbKey, dbs[cdb].Opts.KeySeparator, txCache)
	parentXpath := parentXpathGet(xpath)
	_, ok := xYangSpecMap[xpath]
	if ok && len(xYangSpecMap[xpath].xfmrFunc) > 0 {
		if isFirstCall || (!isFirstCall && (len(xYangSpecMap[parentXpath].xfmrFunc) == 0) ||
			(len(xYangSpecMap[parentXpath].xfmrFunc) > 0 && (xYangSpecMap[parentXpath].xfmrFunc != xYangSpecMap[xpath].xfmrFunc))) {
			xfmrLogInfoAll("Parent subtree already handled cur uri: %v", xpath)
			inParams := formXfmrInputRequest(dbs[cdb], dbs, cdb, ygRoot, curUri, requestUri, GET, dbKey, dbDataMap, nil, nil, txCache)
			err := xfmrHandlerFunc(inParams)
			if err != nil {
				xfmrLogInfoAll("Error returned by %v: %v", xYangSpecMap[xpath].xfmrFunc, err)
			}
		}
		if xYangSpecMap[xpath].hasChildSubTree == true {
			yangDataFill(dbs, ygRoot, curUri, requestUri, xpath, dbDataMap, curMap, tbl, dbKey, cdb, validate, txCache)
		}
	} else {
		_, keyFromCurUri, _, _ := xpathKeyExtract(dbs[cdb], ygRoot, GET, curUri, requestUri, nil, txCache)
		if dbKey == keyFromCurUri || keyFromCurUri == "" {
			if dbKey == keyFromCurUri {
				for k, kv := range curKeyMap {
					curMap[k] = kv
				}
			}
			curXpath, _ := XfmrRemoveXPATHPredicates(curUri)
			yangDataFill(dbs, ygRoot, curUri, requestUri, curXpath, dbDataMap, curMap, tbl, dbKey, cdb, validate, txCache)
		}
	}

	chData.err    = err
	chData.result = curMap
	chl <- chData
	wg.Done()
}

func terminalNodeProcess(dbs [db.MaxDB]*db.DB, ygRoot *ygot.GoStruct, uri string, requestUri string, xpath string, dbDataMap *map[db.DBNum]map[string]map[string]db.Value, tbl string, tblKey string, txCache interface{}) (map[string]interface{}, error) {
	xfmrLogInfoAll("Received xpath - %v, uri - %v, table - %v, table key - %v", xpath, uri, tbl, tblKey)
	var err error
	resFldValMap := make(map[string]interface{})
	_, ok := xYangSpecMap[xpath]
	if !ok || xYangSpecMap[xpath].yangEntry == nil {
		logStr := fmt.Sprintf("No yang entry found for xpath %v.", xpath)
		err = fmt.Errorf("%v", logStr)
		return resFldValMap, err
	}

	cdb := xYangSpecMap[xpath].dbIndex
	if len(xYangSpecMap[xpath].xfmrField) > 0 {
		inParams := formXfmrInputRequest(dbs[cdb], dbs, cdb, ygRoot, uri, requestUri, GET, tblKey, dbDataMap, nil, nil, txCache)
		fldValMap, err := leafXfmrHandlerFunc(inParams)
		if err != nil {
			logStr := fmt.Sprintf("%Failed to get data from overloaded function for %v -v.", uri, err)
			err = fmt.Errorf("%v", logStr)
			return resFldValMap, err
		}
		if fldValMap != nil {
		    for lf, val := range fldValMap {
			resFldValMap[lf] = val
		    }
	        }
	} else {
		dbFldName := xYangSpecMap[xpath].fieldName
		if dbFldName == XFMR_NONE_STRING {
			return resFldValMap, err
		}
		/* if there is no transformer extension/annotation then it means leaf-list in yang is also leaflist in db */
		if len(dbFldName) > 0  && !xYangSpecMap[xpath].isKey {
			yangType := yangTypeGet(xYangSpecMap[xpath].yangEntry)
			yngTerminalNdDtType := xYangSpecMap[xpath].yangEntry.Type.Kind
			if yangType ==  YANG_LEAF_LIST {
				dbFldName += "@"
				val, ok := (*dbDataMap)[cdb][tbl][tblKey].Field[dbFldName]
				if ok {
					resLst := processLfLstDbToYang(xpath, val, yngTerminalNdDtType)
					resFldValMap[xYangSpecMap[xpath].yangEntry.Name] = resLst
				}
			} else {
				val, ok := (*dbDataMap)[cdb][tbl][tblKey].Field[dbFldName]
				if ok {
					resVal, _, err := DbToYangType(yngTerminalNdDtType, xpath, val)
					if err != nil {
						log.Error("Failure in converting Db value type to yang type for field", xpath)
					} else {
						resFldValMap[xYangSpecMap[xpath].yangEntry.Name] = resVal
					}
				}
			}
		}
	}
	return resFldValMap, err
}

func mergeMaps(mapIntfs ...map[string]interface{}) map[string]interface{} {
    resultMap := make(map[string]interface{})
    for _, mapIntf := range mapIntfs {
        for f, v := range mapIntf {
            resultMap[f] = v
        }
    }
    return resultMap
}

func yangDataFill(dbs [db.MaxDB]*db.DB, ygRoot *ygot.GoStruct, uri string, requestUri string, xpath string, dbDataMap *map[db.DBNum]map[string]map[string]db.Value, resultMap map[string]interface{}, tbl string, tblKey string, cdb db.DBNum, validate bool, txCache interface{}) error {
	var err error
	isValid := validate
	yangNode, ok := xYangSpecMap[xpath]

	if ok  && yangNode.yangEntry != nil {
		for yangChldName := range yangNode.yangEntry.Dir {
			chldXpath := xpath+"/"+yangChldName
			chldUri   := uri+"/"+yangChldName
			if xYangSpecMap[chldXpath] != nil && xYangSpecMap[chldXpath].yangEntry != nil {
				cdb = xYangSpecMap[chldXpath].dbIndex
				if len(xYangSpecMap[chldXpath].validateFunc) > 0 && !validate {
					_, key, _, _ := xpathKeyExtract(dbs[cdb], ygRoot, GET, chldUri, requestUri, nil, txCache)
					// TODO - handle non CONFIG-DB
					inParams := formXfmrInputRequest(dbs[cdb], dbs, cdb, ygRoot, chldUri, requestUri, GET, key, dbDataMap, nil, nil, txCache)
					res := validateHandlerFunc(inParams)
					if res != true {
						continue
					} else {
						isValid = res
					}
				}
				chldYangType := xYangSpecMap[chldXpath].yangDataType
				if  chldYangType == YANG_LEAF || chldYangType == YANG_LEAF_LIST {
					if len(xYangSpecMap[xpath].xfmrFunc) > 0 {
						continue
					}
					fldValMap, err := terminalNodeProcess(dbs, ygRoot, chldUri, requestUri, chldXpath, dbDataMap, tbl, tblKey, txCache)
					if err != nil {
						xfmrLogInfoAll("Failed to get data(\"%v\").", chldUri)
					}
					for lf, val := range fldValMap {
						resultMap[lf] = val
					}
				} else if chldYangType == YANG_CONTAINER {
					_, tblKey, chtbl, _ := xpathKeyExtract(dbs[cdb], ygRoot, GET, chldUri, requestUri, nil, txCache)
					if _, ok := (*dbDataMap)[cdb][chtbl]; !ok && len(chtbl) > 0 {
						curDbDataMap, err := fillDbDataMapForTbl(chldUri, chldXpath, chtbl, "", cdb, dbs)
						if err == nil {
							mapCopy((*dbDataMap)[cdb], curDbDataMap[cdb])
						}
					}
					cname := xYangSpecMap[chldXpath].yangEntry.Name
					if xYangSpecMap[chldXpath].xfmrTbl != nil {
						xfmrTblFunc := *xYangSpecMap[chldXpath].xfmrTbl
						if len(xfmrTblFunc) > 0 {
							inParams := formXfmrInputRequest(dbs[cdb], dbs, cdb, ygRoot, chldUri, requestUri, GET, tblKey, dbDataMap, nil, nil, txCache)
							tblList, _ := xfmrTblHandlerFunc(xfmrTblFunc, inParams)
							if len(tblList) > 1 {
								log.Warningf("Table transformer returned more than one table for container %v", chldXpath)
							}
							if len(tblList) == 0 {
								continue
							}
							dbDataFromTblXfmrGet(tblList[0], inParams, dbDataMap)
							chtbl = tblList[0]
						}
					}
					if len(xYangSpecMap[chldXpath].xfmrFunc) > 0 {
						if (len(xYangSpecMap[xpath].xfmrFunc) == 0) ||
						(len(xYangSpecMap[xpath].xfmrFunc) > 0   &&
						(xYangSpecMap[xpath].xfmrFunc != xYangSpecMap[chldXpath].xfmrFunc)) {
							inParams := formXfmrInputRequest(dbs[cdb], dbs, cdb, ygRoot, chldUri, requestUri, GET, "", dbDataMap, nil, nil, txCache)
							err := xfmrHandlerFunc(inParams)
							if err != nil {
								xfmrLogInfoAll("Error returned by %v: %v", xYangSpecMap[xpath].xfmrFunc, err)
							}
						}
						if xYangSpecMap[chldXpath].hasChildSubTree == false {
							continue
						}
					}
					cmap2 := make(map[string]interface{})
					err  = yangDataFill(dbs, ygRoot, chldUri, requestUri, chldXpath, dbDataMap, cmap2, chtbl,
					tblKey, cdb, isValid, txCache)
					if err != nil && len(cmap2) == 0 {
						xfmrLogInfoAll("Empty container.(\"%v\").\r\n", chldUri)
					} else {
						if len(cmap2) > 0 {
							resultMap[cname] = cmap2
						}
					}
				} else if chldYangType ==  YANG_LIST {
					_, tblKey, tbl, _ = xpathKeyExtract(dbs[cdb], ygRoot, GET, chldUri, requestUri, nil, txCache)
					cdb = xYangSpecMap[chldXpath].dbIndex
					if len(xYangSpecMap[chldXpath].xfmrFunc) > 0 {
						if (len(xYangSpecMap[xpath].xfmrFunc) == 0) ||
						   (len(xYangSpecMap[xpath].xfmrFunc) > 0   &&
						   (xYangSpecMap[xpath].xfmrFunc != xYangSpecMap[chldXpath].xfmrFunc)) {
							   inParams := formXfmrInputRequest(dbs[cdb], dbs, cdb, ygRoot, chldUri, requestUri, GET, "", dbDataMap, nil, nil, txCache)
							   err := xfmrHandlerFunc(inParams)
							   if err != nil {
								   xfmrLogInfoAll("Error returned by %v: %v", xYangSpecMap[chldXpath].xfmrFunc, err)
							   }
						}
						if xYangSpecMap[chldXpath].hasChildSubTree == false {
							continue
						}
					}
					ynode, ok := xYangSpecMap[chldXpath]
					lTblName := ""
					if ok && ynode.tableName != nil {
						lTblName = *ynode.tableName
					}
					if _, ok := (*dbDataMap)[cdb][lTblName]; !ok && len(lTblName) > 0 {
						curDbDataMap, err := fillDbDataMapForTbl(chldUri, chldXpath, lTblName, "", cdb, dbs)
						if err == nil {
							mapCopy((*dbDataMap)[cdb], curDbDataMap[cdb])
						}
					}
					yangListDataFill(dbs, ygRoot, chldUri, requestUri, chldXpath, dbDataMap, resultMap, lTblName, tblKey, cdb, isValid, txCache, false)
				} else if chldYangType == "choice" || chldYangType == "case" {
					yangDataFill(dbs, ygRoot, chldUri, requestUri, chldXpath, dbDataMap, resultMap, tbl, tblKey, cdb, isValid, txCache)
				} else {
					return err
				}
			}
		}
	}
	return err
}

/* Traverse linear db-map data and add to nested json data */
func dbDataToYangJsonCreate(uri string, ygRoot *ygot.GoStruct, dbs [db.MaxDB]*db.DB, dbDataMap *map[db.DBNum]map[string]map[string]db.Value, cdb db.DBNum, txCache interface{}) (string, error, bool) {
	var err error
	jsonData := ""
	resultMap := make(map[string]interface{})
	requestUri := uri
	if isSonicYang(uri) {
		return directDbToYangJsonCreate(uri, dbDataMap, resultMap)
	} else {
		var d *db.DB
		reqXpath, keyName, tableName, _ := xpathKeyExtract(d, ygRoot, GET, uri, requestUri, nil, txCache)
		yangNode, ok := xYangSpecMap[reqXpath]
		if ok {
			yangType := yangTypeGet(yangNode.yangEntry)
			validateHandlerFlag := false
			tableXfmrFlag := false
			IsValidate := false
			if len(xYangSpecMap[reqXpath].validateFunc) > 0 {
				inParams := formXfmrInputRequest(dbs[cdb], dbs, cdb, ygRoot, uri, requestUri, GET, keyName, dbDataMap, nil, nil, txCache)
				res := validateHandlerFunc(inParams)
				if !res {
					validateHandlerFlag = true
					/* cannot immediately return from here since reXpath yangtype decides the return type */
				} else {
					IsValidate = res
				}
			}
			isList := false
			switch yangType {
			case YANG_LIST:
				isList = true
			case YANG_LEAF, YANG_LEAF_LIST, YANG_CONTAINER:
				isList = false
			default:
				xfmrLogInfo("Unknown yang object type for path %v", reqXpath)
				isList = true //do not want non-list processing to happen
			}
			/*If yangtype is a list separate code path is to be taken in case of table transformer
			since that code path already handles the calling of table transformer and subsequent processing
			*/
			if (!validateHandlerFlag) && (!isList) {
				if xYangSpecMap[reqXpath].xfmrTbl != nil {
					xfmrTblFunc := *xYangSpecMap[reqXpath].xfmrTbl
					if len(xfmrTblFunc) > 0 {
						inParams := formXfmrInputRequest(dbs[cdb], dbs, cdb, ygRoot, uri, requestUri, GET, keyName, dbDataMap, nil, nil, txCache)
						tblList, _ := xfmrTblHandlerFunc(xfmrTblFunc, inParams)
						if len(tblList) > 1 {
							log.Warningf("Table transformer returned more than one table for container %v", reqXpath)
						}
						if len(tblList) == 0 {
							log.Warningf("Table transformer returned no table for conatiner %v", reqXpath)
							tableXfmrFlag = true
						}
						if !tableXfmrFlag {
                                                      for _, tbl := range tblList {
                                                               dbDataFromTblXfmrGet(tbl, inParams, dbDataMap)
                                                      }

						}
					} else {
						log.Warningf("empty table transformer function name for xpath - %v", reqXpath)
						tableXfmrFlag = true
					}
				}
			}

			for {
				if yangType ==  YANG_LEAF || yangType == YANG_LEAF_LIST {
					yangName := xYangSpecMap[reqXpath].yangEntry.Name
					if validateHandlerFlag || tableXfmrFlag {
						resultMap[yangName] = ""
						break
					}
					if len(xYangSpecMap[reqXpath].xfmrFunc) > 0 {
						inParams := formXfmrInputRequest(dbs[cdb], dbs, cdb, ygRoot, uri, requestUri, GET, "", dbDataMap, nil, nil, txCache)
						err := xfmrHandlerFunc(inParams)
						if err != nil {
							xfmrLogInfo("Error returned by %v: %v", xYangSpecMap[reqXpath].xfmrFunc, err)
						}
					} else {
						tbl, key, _ := tableNameAndKeyFromDbMapGet((*dbDataMap)[cdb])
						fldValMap, err := terminalNodeProcess(dbs, ygRoot, uri, requestUri, reqXpath, dbDataMap, tbl, key, txCache)
						if err != nil {
							xfmrLogInfo("Empty terminal node (\"%v\").", uri)
						}
						resultMap = fldValMap
					}
					break

				} else if yangType == YANG_CONTAINER {
					cmap  := make(map[string]interface{})
					resultMap = cmap
					if validateHandlerFlag || tableXfmrFlag {
						break
					}
					if len(xYangSpecMap[reqXpath].xfmrFunc) > 0 {
						inParams := formXfmrInputRequest(dbs[cdb], dbs, cdb, ygRoot, uri, requestUri, GET, "", dbDataMap, nil, nil, txCache)
						err := xfmrHandlerFunc(inParams)
						if err != nil {
							xfmrLogInfo("Error returned by %v: %v", xYangSpecMap[reqXpath].xfmrFunc, err)
						}
						if xYangSpecMap[reqXpath].hasChildSubTree == false {
							break
						}
					}
					err = yangDataFill(dbs, ygRoot, uri, requestUri, reqXpath, dbDataMap, resultMap, tableName, keyName, cdb, IsValidate, txCache)
					if err != nil {
						xfmrLogInfo("Empty container(\"%v\").\r\n", uri)
					}
					break
				} else if yangType == YANG_LIST {
					isFirstCall := true
					if len(xYangSpecMap[reqXpath].xfmrFunc) > 0 {
						inParams := formXfmrInputRequest(dbs[cdb], dbs, cdb, ygRoot, uri, requestUri, GET, "", dbDataMap, nil, nil, txCache)
						err := xfmrHandlerFunc(inParams)
						if err != nil {
							xfmrLogInfo("Error returned by %v: %v", xYangSpecMap[reqXpath].xfmrFunc, err)
						}
						isFirstCall = false
						if xYangSpecMap[reqXpath].hasChildSubTree == false {
							break
						}
					}
					err = yangListDataFill(dbs, ygRoot, uri, requestUri, reqXpath, dbDataMap, resultMap, tableName, keyName, cdb, IsValidate, txCache, isFirstCall)
					if err != nil {
						xfmrLogInfo("yangListDataFill failed for list case(\"%v\").\r\n", uri)
					}
					break
				} else {
					log.Warningf("Unknown yang object type for path %v", reqXpath)
					break
				}
			} //end of for
		}
	}

	jsonMapData, _ := json.Marshal(resultMap)
	isEmptyPayload := isJsonDataEmpty(string(jsonMapData))
	jsonData        = fmt.Sprintf("%v", string(jsonMapData))

	return jsonData, nil, isEmptyPayload
}
