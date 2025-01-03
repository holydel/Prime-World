/**
 * Autogenerated by Thrift Compiler (0.9.1)
 *
 * DO NOT EDIT UNLESS YOU ARE SURE THAT YOU KNOW WHAT YOU ARE DOING
 *  @generated
 */
using System;
using System.Collections;
using System.Collections.Generic;
using System.Text;
using System.IO;
using Thrift;
using Thrift.Collections;
using System.Runtime.Serialization;
using Thrift.Protocol;
using Thrift.Transport;

namespace AccountLib
{

  #if !SILVERLIGHT
  [Serializable]
  #endif
  public partial class RerollShopGroupSlots : TBase
  {
    private List<string> _persistentIds;
    private string _groupId;

    public List<string> PersistentIds
    {
      get
      {
        return _persistentIds;
      }
      set
      {
        __isset.persistentIds = true;
        this._persistentIds = value;
      }
    }

    public string GroupId
    {
      get
      {
        return _groupId;
      }
      set
      {
        __isset.groupId = true;
        this._groupId = value;
      }
    }


    public Isset __isset;
    #if !SILVERLIGHT
    [Serializable]
    #endif
    public struct Isset {
      public bool persistentIds;
      public bool groupId;
    }

    public RerollShopGroupSlots() {
    }

    public void Read (TProtocol iprot)
    {
      TField field;
      iprot.ReadStructBegin();
      while (true)
      {
        field = iprot.ReadFieldBegin();
        if (field.Type == TType.Stop) { 
          break;
        }
        switch (field.ID)
        {
          case 1:
            if (field.Type == TType.List) {
              {
                PersistentIds = new List<string>();
                TList _list338 = iprot.ReadListBegin();
                for( int _i339 = 0; _i339 < _list338.Count; ++_i339)
                {
                  string _elem340 = null;
                  _elem340 = iprot.ReadString();
                  PersistentIds.Add(_elem340);
                }
                iprot.ReadListEnd();
              }
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 2:
            if (field.Type == TType.String) {
              GroupId = iprot.ReadString();
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          default: 
            TProtocolUtil.Skip(iprot, field.Type);
            break;
        }
        iprot.ReadFieldEnd();
      }
      iprot.ReadStructEnd();
    }

    public void Write(TProtocol oprot) {
      TStruct struc = new TStruct("RerollShopGroupSlots");
      oprot.WriteStructBegin(struc);
      TField field = new TField();
      if (PersistentIds != null && __isset.persistentIds) {
        field.Name = "persistentIds";
        field.Type = TType.List;
        field.ID = 1;
        oprot.WriteFieldBegin(field);
        {
          oprot.WriteListBegin(new TList(TType.String, PersistentIds.Count));
          foreach (string _iter341 in PersistentIds)
          {
            oprot.WriteString(_iter341);
          }
          oprot.WriteListEnd();
        }
        oprot.WriteFieldEnd();
      }
      if (GroupId != null && __isset.groupId) {
        field.Name = "groupId";
        field.Type = TType.String;
        field.ID = 2;
        oprot.WriteFieldBegin(field);
        oprot.WriteString(GroupId);
        oprot.WriteFieldEnd();
      }
      oprot.WriteFieldStop();
      oprot.WriteStructEnd();
    }

    public override string ToString() {
      StringBuilder sb = new StringBuilder("RerollShopGroupSlots(");
      sb.Append("PersistentIds: ");
      sb.Append(PersistentIds);
      sb.Append(",GroupId: ");
      sb.Append(GroupId);
      sb.Append(")");
      return sb.ToString();
    }

  }

}
