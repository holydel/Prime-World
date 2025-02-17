# -*- coding: utf-8 -*-
# Automatically generated file. do not modify it!!!

import sys, os
from base.helpers import addRelPath
from modeldata.baseclasses import *
from modeldata.changes import *
from modeldata.collects import *
from modeldata.ref import *

addRelPath("../modeldata", "../modeldata/SharedTypes")

import GuildShortInfo

from GuildMapEntity_base import *

class GuildMapEntity(RefCounted, Identified, ChangeHandler, BaseObject, GuildMapEntity_base):
    _changeFields = {
        "DonationToday":1,
        "DonationYesterday":1,
        "JoinTime":1,
        "ShortInfo":1,
    }

    def __init__(self, modeldata, id=None, path=None):
        ChangeHandler.__init__(self, path, modeldata)
        RefCounted.__init__(self)
        Identified.__init__(self, id)
        _dict = self.__dict__
        _dict["_modeldata"] = modeldata
        _dict["DonationToday"] = 0
        _dict["DonationYesterday"] = 0
        _dict["JoinTime"] = 0
        _dict["ShortInfo"] = Ref(modeldata, modeldata.GuildShortInfoKeeper, path, "ShortInfo")
        _dict["isDeleting"] = False

    # ���������� �� ���� ������� ����� model.addNewName() -- ��������, � keeper.init() ����� ��������
    def init(self):
        pass
        ###self.ShortInfo.init(self._modeldata.GuildShortInfoKeeper, self.path, self._modeldata, "ShortInfo")

    # ���������� ����� model.addNewName()
    def init_add(self):
        pass
        ###self.ShortInfo.init(self._modeldata.GuildShortInfoKeeper, self.path, self._modeldata, "ShortInfo")

    def setPath(self, path):
        ChangeHandler.init(self, path, self._modeldata)
        self.ShortInfo.init(self._modeldata.GuildShortInfoKeeper, self.path, self._modeldata, "ShortInfo")

    def deleteByID(self, id):
        if not self.isDeleting:
            self.isDeleting = True
            self.ShortInfo.deleteByID(id)
            self.isDeleting = False

    def __setattr__(self, name, val):
        if name == "ShortInfo":
            self.__dict__["ShortInfo"].set(val)
            return
        ChangeHandler.__setattr__(self, name, val)


    def generateJsonDict(self):
        json_dict = GuildMapEntity_base.generateBaseDict(self)
        json_dict['id'] = self.id
        json_dict['path'] = self.path
        json_dict['refCounter'] = self.refCounter
        _dict = self.__dict__
        json_dict["DonationToday"]=_dict.get("DonationToday")
        json_dict["DonationYesterday"]=_dict.get("DonationYesterday")
        json_dict["JoinTime"]=_dict.get("JoinTime")
        json_dict["ShortInfo"]=_dict.get("ShortInfo").getJsonDict()
        return {"GuildMapEntity": json_dict}

if not hasattr(GuildMapEntity_base, "generateBaseDict"):
    GuildMapEntity_base.generateBaseDict = generateEmptyDict
