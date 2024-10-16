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
  public partial class SeasonInfo : TBase
  {
    private int _seasonId;
    private string _seasonName;
    private long _startDate;
    private long _endDate;
    private int _seasonRating;
    private int _leagueIndex;
    private int _curLeaguePlace;
    private int _bestLeaguePlace;

    public int SeasonId
    {
      get
      {
        return _seasonId;
      }
      set
      {
        __isset.seasonId = true;
        this._seasonId = value;
      }
    }

    public string SeasonName
    {
      get
      {
        return _seasonName;
      }
      set
      {
        __isset.seasonName = true;
        this._seasonName = value;
      }
    }

    public long StartDate
    {
      get
      {
        return _startDate;
      }
      set
      {
        __isset.startDate = true;
        this._startDate = value;
      }
    }

    public long EndDate
    {
      get
      {
        return _endDate;
      }
      set
      {
        __isset.endDate = true;
        this._endDate = value;
      }
    }

    public int SeasonRating
    {
      get
      {
        return _seasonRating;
      }
      set
      {
        __isset.seasonRating = true;
        this._seasonRating = value;
      }
    }

    public int LeagueIndex
    {
      get
      {
        return _leagueIndex;
      }
      set
      {
        __isset.leagueIndex = true;
        this._leagueIndex = value;
      }
    }

    public int CurLeaguePlace
    {
      get
      {
        return _curLeaguePlace;
      }
      set
      {
        __isset.curLeaguePlace = true;
        this._curLeaguePlace = value;
      }
    }

    public int BestLeaguePlace
    {
      get
      {
        return _bestLeaguePlace;
      }
      set
      {
        __isset.bestLeaguePlace = true;
        this._bestLeaguePlace = value;
      }
    }


    public Isset __isset;
    #if !SILVERLIGHT
    [Serializable]
    #endif
    public struct Isset {
      public bool seasonId;
      public bool seasonName;
      public bool startDate;
      public bool endDate;
      public bool seasonRating;
      public bool leagueIndex;
      public bool curLeaguePlace;
      public bool bestLeaguePlace;
    }

    public SeasonInfo() {
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
              SeasonId = iprot.ReadI32();
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 2:
            if (field.Type == TType.String) {
              SeasonName = iprot.ReadString();
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 3:
            if (field.Type == TType.I64) {
              StartDate = iprot.ReadI64();
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 4:
            if (field.Type == TType.I64) {
              EndDate = iprot.ReadI64();
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 5:
            if (field.Type == TType.I32) {
              SeasonRating = iprot.ReadI32();
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 6:
            if (field.Type == TType.I32) {
              LeagueIndex = iprot.ReadI32();
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 7:
            if (field.Type == TType.I32) {
              CurLeaguePlace = iprot.ReadI32();
            } else { 
              TProtocolUtil.Skip(iprot, field.Type);
            }
            break;
          case 8:
            if (field.Type == TType.I32) {
              BestLeaguePlace = iprot.ReadI32();
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
      TStruct struc = new TStruct("SeasonInfo");
      oprot.WriteStructBegin(struc);
      TField field = new TField();
      if (__isset.seasonId) {
        field.Name = "seasonId";
        field.Type = TType.I32;
        field.ID = 1;
        oprot.WriteFieldBegin(field);
        oprot.WriteI32(SeasonId);
        oprot.WriteFieldEnd();
      }
      if (SeasonName != null && __isset.seasonName) {
        field.Name = "seasonName";
        field.Type = TType.String;
        field.ID = 2;
        oprot.WriteFieldBegin(field);
        oprot.WriteString(SeasonName);
        oprot.WriteFieldEnd();
      }
      if (__isset.startDate) {
        field.Name = "startDate";
        field.Type = TType.I64;
        field.ID = 3;
        oprot.WriteFieldBegin(field);
        oprot.WriteI64(StartDate);
        oprot.WriteFieldEnd();
      }
      if (__isset.endDate) {
        field.Name = "endDate";
        field.Type = TType.I64;
        field.ID = 4;
        oprot.WriteFieldBegin(field);
        oprot.WriteI64(EndDate);
        oprot.WriteFieldEnd();
      }
      if (__isset.seasonRating) {
        field.Name = "seasonRating";
        field.Type = TType.I32;
        field.ID = 5;
        oprot.WriteFieldBegin(field);
        oprot.WriteI32(SeasonRating);
        oprot.WriteFieldEnd();
      }
      if (__isset.leagueIndex) {
        field.Name = "leagueIndex";
        field.Type = TType.I32;
        field.ID = 6;
        oprot.WriteFieldBegin(field);
        oprot.WriteI32(LeagueIndex);
        oprot.WriteFieldEnd();
      }
      if (__isset.curLeaguePlace) {
        field.Name = "curLeaguePlace";
        field.Type = TType.I32;
        field.ID = 7;
        oprot.WriteFieldBegin(field);
        oprot.WriteI32(CurLeaguePlace);
        oprot.WriteFieldEnd();
      }
      if (__isset.bestLeaguePlace) {
        field.Name = "bestLeaguePlace";
        field.Type = TType.I32;
        field.ID = 8;
        oprot.WriteFieldBegin(field);
        oprot.WriteI32(BestLeaguePlace);
        oprot.WriteFieldEnd();
      }
      oprot.WriteFieldStop();
      oprot.WriteStructEnd();
    }

    public override string ToString() {
      StringBuilder sb = new StringBuilder("SeasonInfo(");
      sb.Append("SeasonId: ");
      sb.Append(SeasonId);
      sb.Append(",SeasonName: ");
      sb.Append(SeasonName);
      sb.Append(",StartDate: ");
      sb.Append(StartDate);
      sb.Append(",EndDate: ");
      sb.Append(EndDate);
      sb.Append(",SeasonRating: ");
      sb.Append(SeasonRating);
      sb.Append(",LeagueIndex: ");
      sb.Append(LeagueIndex);
      sb.Append(",CurLeaguePlace: ");
      sb.Append(CurLeaguePlace);
      sb.Append(",BestLeaguePlace: ");
      sb.Append(BestLeaguePlace);
      sb.Append(")");
      return sb.ToString();
    }

  }

}
