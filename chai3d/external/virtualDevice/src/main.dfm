object Form1: TForm1
  Left = 717
  Top = 447
  BorderStyle = bsDialog
  Caption = 'Virtual Haptic Device'
  ClientHeight = 402
  ClientWidth = 215
  Color = 5460819
  Font.Charset = DEFAULT_CHARSET
  Font.Color = clWindowText
  Font.Height = -11
  Font.Name = 'MS Sans Serif'
  Font.Style = []
  FormStyle = fsStayOnTop
  OldCreateOrder = False
  Position = poDesktopCenter
  OnCreate = FormCreate
  PixelsPerInch = 96
  TextHeight = 13
  object Label1: TLabel
    Left = 8
    Top = 8
    Width = 57
    Height = 16
    Caption = 'Position'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWhite
    Font.Height = -15
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label2: TLabel
    Left = 136
    Top = 8
    Width = 41
    Height = 16
    Caption = 'Force'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWhite
    Font.Height = -15
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label3: TLabel
    Left = 8
    Top = 112
    Width = 114
    Height = 16
    Caption = 'Force Amplitude'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWhite
    Font.Height = -15
    Font.Name = 'MS Sans Serif'
    Font.Style = [fsBold]
    ParentFont = False
  end
  object Label4: TLabel
    Left = 8
    Top = 352
    Width = 40
    Height = 13
    Caption = 'LButton:'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWhite
    Font.Height = -12
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label5: TLabel
    Left = 8
    Top = 384
    Width = 42
    Height = 13
    Caption = 'RButton:'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWhite
    Font.Height = -12
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label6: TLabel
    Left = 104
    Top = 352
    Width = 37
    Height = 13
    Caption = 'YZ plan'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWhite
    Font.Height = -12
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label7: TLabel
    Left = 104
    Top = 384
    Width = 37
    Height = 13
    Caption = 'XY plan'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWhite
    Font.Height = -12
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label8: TLabel
    Left = 104
    Top = 368
    Width = 37
    Height = 13
    Caption = 'XY plan'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWhite
    Font.Height = -12
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label9: TLabel
    Left = 8
    Top = 368
    Width = 80
    Height = 13
    Caption = 'LButton + SHIFT'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWhite
    Font.Height = -12
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label10: TLabel
    Left = 64
    Top = 36
    Width = 14
    Height = 13
    Caption = '[m]'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clYellow
    Font.Height = -12
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label11: TLabel
    Left = 64
    Top = 60
    Width = 14
    Height = 13
    Caption = '[m]'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clYellow
    Font.Height = -12
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label12: TLabel
    Left = 64
    Top = 84
    Width = 14
    Height = 13
    Caption = '[m]'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clYellow
    Font.Height = -12
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label13: TLabel
    Left = 192
    Top = 60
    Width = 14
    Height = 13
    Caption = '[N]'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clYellow
    Font.Height = -12
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label14: TLabel
    Left = 192
    Top = 84
    Width = 14
    Height = 13
    Caption = '[N]'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clYellow
    Font.Height = -12
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object Label15: TLabel
    Left = 192
    Top = 36
    Width = 14
    Height = 13
    Caption = '[N]'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clYellow
    Font.Height = -12
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
  end
  object edtPosX: TEdit
    Left = 8
    Top = 32
    Width = 49
    Height = 21
    ReadOnly = True
    TabOrder = 0
  end
  object edtPosY: TEdit
    Left = 8
    Top = 56
    Width = 49
    Height = 21
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clWindowText
    Font.Height = -12
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    ReadOnly = True
    TabOrder = 1
  end
  object edtPosZ: TEdit
    Left = 8
    Top = 80
    Width = 49
    Height = 21
    ReadOnly = True
    TabOrder = 2
  end
  object upDownZ: TUpDown
    Left = 88
    Top = 80
    Width = 13
    Height = 19
    Min = -30000
    Max = 30000
    Position = 0
    TabOrder = 3
    Wrap = False
    OnClick = upDownZClick
  end
  object upDownX: TUpDown
    Left = 88
    Top = 32
    Width = 13
    Height = 19
    Min = -30000
    Max = 30000
    Position = 0
    TabOrder = 4
    Wrap = False
    OnClick = upDownXClick
  end
  object upDownY: TUpDown
    Left = 88
    Top = 56
    Width = 13
    Height = 19
    Min = -30000
    Max = 30000
    Position = 0
    TabOrder = 5
    Wrap = False
    OnClick = upDownYClick
  end
  object edtForceZ: TEdit
    Left = 136
    Top = 80
    Width = 49
    Height = 21
    ReadOnly = True
    TabOrder = 6
  end
  object edtForceY: TEdit
    Left = 136
    Top = 56
    Width = 49
    Height = 21
    ReadOnly = True
    TabOrder = 7
  end
  object edtForceX: TEdit
    Left = 136
    Top = 32
    Width = 49
    Height = 21
    ReadOnly = True
    TabOrder = 8
  end
  object panelView: TPanel
    Left = 0
    Top = 160
    Width = 216
    Height = 185
    TabOrder = 9
    OnMouseDown = panelViewMouseDown
    OnMouseMove = panelViewMouseMove
    OnMouseUp = panelViewMouseUp
  end
  object barForce: TProgressBar
    Left = 8
    Top = 136
    Width = 201
    Height = 12
    Min = 0
    Max = 100
    ParentShowHint = False
    ShowHint = False
    TabOrder = 10
  end
  object upDownXs: TUpDown
    Left = 104
    Top = 32
    Width = 13
    Height = 19
    Min = -30000
    Max = 30000
    Position = 0
    TabOrder = 11
    Wrap = False
    OnClick = upDownXsClick
  end
  object upDownYs: TUpDown
    Left = 104
    Top = 56
    Width = 13
    Height = 19
    Min = -30000
    Max = 30000
    Position = 0
    TabOrder = 12
    Wrap = False
    OnClick = upDownYsClick
  end
  object upDownZs: TUpDown
    Left = 104
    Top = 80
    Width = 13
    Height = 19
    Min = -30000
    Max = 30000
    Position = 0
    TabOrder = 13
    Wrap = False
    OnClick = upDownZsClick
  end
  object ckbButton: TCheckBox
    Left = 136
    Top = 112
    Width = 73
    Height = 17
    Caption = 'Switch'
    Font.Charset = DEFAULT_CHARSET
    Font.Color = clYellow
    Font.Height = -15
    Font.Name = 'MS Sans Serif'
    Font.Style = []
    ParentFont = False
    TabOrder = 14
  end
  object Timer1: TTimer
    Interval = 50
    OnTimer = Timer1Timer
    Left = 8
    Top = 168
  end
end
