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
  public partial class RerollShopGroupCurrencies : TBase
  {
    private string _persistentId;
    private int _currencies;

    public string PersistentId
    {
      get
      {
        return _persistentId;
      }
      set
      {
        __isset.persistentId = true;
        this._persistentId = value;
      }
    }

    public int Currencies
    {
      get
      {
        return _currencies;
      }
      set
      {
        __isset.currencies = true;
        this._currencies = value;
      }
    }


    public Isset __isset;
    #if !SILVERLIGHT
    [Serializable]
    #endif
    public struct Isset {
      public bool persistentId;
      public bool currencies;
    }

    public RerollShopGroupCurrencies() {
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
            if (field.Type == TType.String) {
              PersistentId = iprot.ReadString();
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 2:
            if (field.Type == TType.I32) {
              Currencies = iprot.ReadI32();
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
      TStruct struc = new TStruct("RerollShopGroupCurrencies");
      oprot.WriteStructBegin(struc);
      TField field = new TField();
      if (PersistentId != null && __isset.persistentId) {
        field.Name = "persistentId";
        field.Type = TType.String;
        field.ID = 1;
        oprot.WriteFieldBegin(field);
        oprot.WriteString(PersistentId);
        oprot.WriteFieldEnd();
      }
      if (__isset.currencies) {
        field.Name = "currencies";
        field.Type = TType.I32;
        field.ID = 2;
        oprot.WriteFieldBegin(field);
        oprot.WriteI32(Currencies);
        oprot.WriteFieldEnd();
      }
      oprot.WriteFieldStop();
      oprot.WriteStructEnd();
    }

    public override string ToString() {
      StringBuilder sb = new StringBuilder("RerollShopGroupCurrencies(");
      sb.Append("PersistentId: ");
      sb.Append(PersistentId);
      sb.Append(",Currencies: ");
      sb.Append(Currencies);
      sb.Append(")");
      return sb.ToString();
    }

  }

}
