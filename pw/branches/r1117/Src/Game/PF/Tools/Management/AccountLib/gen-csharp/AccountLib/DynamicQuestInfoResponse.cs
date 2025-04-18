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
  public partial class DynamicQuestInfoResponse : TBase
  {
    private RequestResult _result;
    private List<DynamicQuestInfo> _quests;

    /// <summary>
    /// 
    /// <seealso cref="RequestResult"/>
    /// </summary>
    public RequestResult Result
    {
      get
      {
        return _result;
      }
      set
      {
        __isset.result = true;
        this._result = value;
      }
    }

    public List<DynamicQuestInfo> Quests
    {
      get
      {
        return _quests;
      }
      set
      {
        __isset.quests = true;
        this._quests = value;
      }
    }


    public Isset __isset;
    #if !SILVERLIGHT
    [Serializable]
    #endif
    public struct Isset {
      public bool result;
      public bool quests;
    }

    public DynamicQuestInfoResponse() {
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
            if (field.Type == TType.I32) {
              Result = (RequestResult)iprot.ReadI32();
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 2:
            if (field.Type == TType.List) {
              {
                Quests = new List<DynamicQuestInfo>();
                TList _list114 = iprot.ReadListBegin();
                for( int _i115 = 0; _i115 < _list114.Count; ++_i115)
                {
                  DynamicQuestInfo _elem116 = new DynamicQuestInfo();
                  _elem116 = new DynamicQuestInfo();
                  _elem116.Read(iprot);
                  Quests.Add(_elem116);
                }
                iprot.ReadListEnd();
              }
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
      TStruct struc = new TStruct("DynamicQuestInfoResponse");
      oprot.WriteStructBegin(struc);
      TField field = new TField();
      if (__isset.result) {
        field.Name = "result";
        field.Type = TType.I32;
        field.ID = 1;
        oprot.WriteFieldBegin(field);
        oprot.WriteI32((int)Result);
        oprot.WriteFieldEnd();
      }
      if (Quests != null && __isset.quests) {
        field.Name = "quests";
        field.Type = TType.List;
        field.ID = 2;
        oprot.WriteFieldBegin(field);
        {
          oprot.WriteListBegin(new TList(TType.Struct, Quests.Count));
          foreach (DynamicQuestInfo _iter117 in Quests)
          {
            _iter117.Write(oprot);
          }
          oprot.WriteListEnd();
        }
        oprot.WriteFieldEnd();
      }
      oprot.WriteFieldStop();
      oprot.WriteStructEnd();
    }

    public override string ToString() {
      StringBuilder sb = new StringBuilder("DynamicQuestInfoResponse(");
      sb.Append("Result: ");
      sb.Append(Result);
      sb.Append(",Quests: ");
      sb.Append(Quests);
      sb.Append(")");
      return sb.ToString();
    }

  }

}
