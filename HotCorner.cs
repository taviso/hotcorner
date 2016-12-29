using System;
using System.Threading;
using System.Windows.Forms;
using System.Runtime.InteropServices;

//Original source: https://github.com/taviso/hotcorner
//Converted to C# by Etor Madiv
//https://www.virustotal.com/en/file/a6badd92b937ef9cd9911a14958f8e695012c3e004a8aba5c774fbcabf449ece/analysis/1482975853/

namespace HotCornerApp
{
	public class Program
	{
		[DllImport("user32.dll", SetLastError = true)]
		static extern IntPtr SetWindowsHookEx(HookType hookType, HookProc lpfn, IntPtr hMod, uint dwThreadId);
		
		[DllImport("user32.dll", SetLastError = true)]
		[return: MarshalAs(UnmanagedType.Bool)]
		static extern bool RegisterHotKey(IntPtr hWnd, int id, ModifierKeys fsModifiers, Keys vk);
		
		[DllImport("user32.dll")]
		static extern bool PtInRect([In] ref RECT lprc, POINT pt);
		
		[DllImport("kernel32.dll")]
		static extern bool TerminateThread(IntPtr hThread, uint dwExitCode);
		
		[DllImport("USER32.dll")]
		static extern short GetKeyState(VirtualKeyStates nVirtKey);
		
		[DllImport("user32.dll")]
		static extern IntPtr CallNextHookEx(IntPtr hhk, int nCode, int wParam, [In]MSLLHOOKSTRUCT lParam);
		
		[DllImport("user32.dll", SetLastError = true)]
		[return: MarshalAs(UnmanagedType.Bool)]
		static extern bool UnhookWindowsHookEx(IntPtr hhk);
		
		[DllImport("user32.dll", SetLastError = true)]
		[return: MarshalAs(UnmanagedType.Bool)]
		static extern bool GetCursorPos(out POINT lpPoint);
		
		[DllImport("user32.dll")]
		static extern uint SendInput(int nInputs, [MarshalAs(UnmanagedType.LPArray), In] INPUT[] pInputs, int cbSize);
		
		[DllImport("Kernel32.dll")]
        public static extern IntPtr CreateThread(IntPtr SecurityAttributes,
            uint StackSize, CreateThreadProc StartFunction, IntPtr ThreadParameter,
            uint CreationFlags, IntPtr ThreadId);
		
		[DllImport("Kernel32.dll", SetLastError = true)]
        public static extern bool CloseHandle(IntPtr hObject);
		
		private static CreateThreadProc CornerHotFuncPtr = new CreateThreadProc(CornerHotFunc);
		
		private static int WM_MOUSEMOVE = 0x200;
		
		private static RECT kHotCorner = new RECT(
			-20, //Left
			-20, //Top
			+20, //Right
			+20  //Bottom
		);
		
		// Input to inject when corner activated (Win+Tab by default).
		private static INPUT[] kCornerInput = new INPUT[]{
			new INPUT{
				type = InputType.KEYBOARD, 
				U = new InputUnion{
					ki = new KEYBDINPUT{
						wVk = VirtualKeyShort.LWIN,
						dwFlags = KEYEVENTF.NONE
					}
				}
			},
			new INPUT{
				type = InputType.KEYBOARD, 
				U = new InputUnion{
					ki = new KEYBDINPUT{
						wVk = VirtualKeyShort.TAB,
						dwFlags = KEYEVENTF.NONE
					}
				}
			},
			new INPUT{
				type = InputType.KEYBOARD, 
				U = new InputUnion{
					ki = new KEYBDINPUT{
						wVk = VirtualKeyShort.TAB,
						dwFlags = KEYEVENTF.KEYUP
					}
				}
			},
			new INPUT{
				type = InputType.KEYBOARD, 
				U = new InputUnion{
					ki = new KEYBDINPUT{
						wVk = VirtualKeyShort.LWIN,
						dwFlags = KEYEVENTF.KEYUP
					}
				}
			}
		};
		
		private static int kHotDelay = 300;
		
		private static IntPtr CornerThread;
		
		public static void Main()
		{
			HookProc mouseHookCallback = new HookProc(MouseHookCallback);
			IntPtr MouseHook = SetWindowsHookEx(HookType.WH_MOUSE_LL, mouseHookCallback, IntPtr.Zero, 0);
			
			if(MouseHook == IntPtr.Zero)
			{
				return;
			}
			
			ModifierKeys kHotKeyModifiers = ModifierKeys.MOD_CONTROL | ModifierKeys.MOD_ALT;
			Keys kHotKey                  = Keys.C;
			
			if(!RegisterHotKey(IntPtr.Zero, 1, kHotKeyModifiers, kHotKey))
			{
				return;
			}
			
			HotCornerMessageFilter hotCornerMessageFilter = new HotCornerMessageFilter();
			
			Application.AddMessageFilter(hotCornerMessageFilter);
			
			Application.Run();
			
			Application.RemoveMessageFilter(hotCornerMessageFilter);
			
			UnhookWindowsHookEx(MouseHook);
		}
		
		public static void CornerHotFunc(IntPtr pData)
		{
			POINT Point;
			
			Thread.Sleep(kHotDelay);
			
			if (GetKeyState(VirtualKeyStates.LBUTTON) < 0 || GetKeyState(VirtualKeyStates.RBUTTON) < 0) {
				return;
			}
			
			// Verify the corner is still hot
			if (GetCursorPos(out Point) == false) {
				return;
			}
			
			// Check co-ordinates.
			if (PtInRect(ref kHotCorner, Point)) {
				SendInput(kCornerInput.Length, kCornerInput, INPUT.Size);
			}
		}
		
		public static IntPtr MouseHookCallback(int code, int wParam, ref MSLLHOOKSTRUCT evt)
		{
			// If the mouse hasn't moved, we're done.
			if (wParam != WM_MOUSEMOVE)
				goto finish;
			
			// Check if the cursor is hot or cold.
			if (!PtInRect(ref kHotCorner, evt.pt)) {

				// The corner is cold, and was cold before.
				if (CornerThread == IntPtr.Zero)
					goto finish;

				// The corner is cold, but was previously hot.
				TerminateThread(CornerThread, 0);

				CloseHandle(CornerThread);
				
				// Reset state.
				CornerThread = IntPtr.Zero;

				goto finish;
			}
			
			// The corner is hot, check if it was already hot.
			if (CornerThread != IntPtr.Zero) {
				goto finish;
			}
			
			// Check if a mouse putton is pressed, maybe a drag operation?
			if (GetKeyState(VirtualKeyStates.LBUTTON) < 0 || GetKeyState(VirtualKeyStates.RBUTTON) < 0) {
				goto finish;
			}
			
			CornerThread = CreateThread(IntPtr.Zero, 0, CornerHotFuncPtr, IntPtr.Zero, 0, IntPtr.Zero);
			
		finish:
			return CallNextHookEx(IntPtr.Zero, code, wParam, evt);			
		}
	}
	
	public enum HookType : int
	{
		 WH_JOURNALRECORD = 0,  
		 WH_JOURNALPLAYBACK = 1,
		 WH_KEYBOARD = 2,
		 WH_GETMESSAGE = 3,
		 WH_CALLWNDPROC = 4,
		 WH_CBT = 5,
		 WH_SYSMSGFILTER = 6,
		 WH_MOUSE = 7,
		 WH_HARDWARE = 8,
		 WH_DEBUG = 9,
		 WH_SHELL = 10,
		 WH_FOREGROUNDIDLE = 11,
		 WH_CALLWNDPROCRET = 12,
		 WH_KEYBOARD_LL = 13,
		 WH_MOUSE_LL = 14
	}
	
	[Flags]
	public enum ModifierKeys : uint
	{
		MOD_ALT = 0x0001,
		MOD_CONTROL = 0x0002,
		MOD_SHIFT = 0x0004,
		MOD_WIN = 0x0008
	}
	
	public enum VirtualKeyStates : int
    {
        LBUTTON = 0x01,
        RBUTTON = 0x02
	}
	
	[StructLayout(LayoutKind.Sequential)]
	public struct SECURITY_ATTRIBUTES
	{
		public int nLength;
		public IntPtr lpSecurityDescriptor;
		public int bInheritHandle;
	}
	
	[StructLayout(LayoutKind.Sequential)]
	public struct INPUT
	{
		public InputType type;
		public InputUnion U;
		public static int Size
		{
			get { return Marshal.SizeOf(typeof(INPUT)); }
		}
	}

	[StructLayout(LayoutKind.Explicit)]
	public struct InputUnion
	{
		[FieldOffset(0)]
		public MOUSEINPUT mi;
		[FieldOffset(0)]
		public KEYBDINPUT ki;
		[FieldOffset(0)]
		public HARDWAREINPUT hi;
	}
	
	public enum InputType : uint
	{
		MOUSE    = 0,
		KEYBOARD = 1,
		HARDWARE = 2
	}
	
	[StructLayout(LayoutKind.Sequential)]
    public struct MOUSEINPUT
    {
        public int dx;
        public int dy;
        public int mouseData;
        public uint dwFlags;
        public uint time;
        public UIntPtr dwExtraInfo;
    }
	
	[StructLayout(LayoutKind.Sequential)]
    public struct KEYBDINPUT
    {
        public VirtualKeyShort wVk;
        public short wScan;
        public KEYEVENTF dwFlags;
        public int time;
        public UIntPtr dwExtraInfo;
    }
	
	[StructLayout(LayoutKind.Sequential)]
    public struct HARDWAREINPUT
    {
        public int   uMsg;
        public short wParamL;
        public short wParamH;
    }
	
	public enum VirtualKeyShort : short
	{
		TAB = 0x09,
		LWIN = 0x5B
	}
	
	[Flags]
    public enum KEYEVENTF : uint
    {
        NONE = 0x0000,
        EXTENDEDKEY = 0x0001,
        KEYUP = 0x0002,
        SCANCODE = 0x0008,
        UNICODE = 0x0004
    }
	
	[StructLayout(LayoutKind.Sequential)]
	public struct MSLLHOOKSTRUCT
	{
		public POINT pt;
		public int mouseData; // be careful, this must be ints, not uints (was wrong before I changed it...). regards, cmew.
		public int flags;
		public int time;
		public UIntPtr dwExtraInfo;
	}
	
	[StructLayout(LayoutKind.Sequential)]
    public struct POINT
    {
        public int X;
        public int Y;

        public POINT(int x, int y)
        {
            this.X = x;
            this.Y = y;
        }

        public POINT(System.Drawing.Point pt) : this(pt.X, pt.Y) { }

        public static implicit operator System.Drawing.Point(POINT p)
        {
            return new System.Drawing.Point(p.X, p.Y);
        }

        public static implicit operator POINT(System.Drawing.Point p)
        {
            return new POINT(p.X, p.Y);
        }
    }
	
	[StructLayout(LayoutKind.Sequential)]
	public struct RECT
	{
	   public int Left, Top, Right, Bottom;

	   public RECT(int left, int top, int right, int bottom)
	   {
		 Left = left;
		 Top = top;
		 Right = right;
		 Bottom = bottom;
	   }

	   public RECT(System.Drawing.Rectangle r) : this(r.Left, r.Top, r.Right, r.Bottom) { }

	   public int X
	   {
		 get { return Left; }
		 set { Right -= (Left - value); Left = value; }
	   }

	   public int Y
	   {
		 get { return Top; }
		 set { Bottom -= (Top - value); Top = value; }
	   }

	   public int Height
	   {
		 get { return Bottom - Top; }
		 set { Bottom = value + Top; }
	   }

	   public int Width
	   {
		 get { return Right - Left; }
		 set { Right = value + Left; }
	   }

	   public System.Drawing.Point Location
	   {
		 get { return new System.Drawing.Point(Left, Top); }
		 set { X = value.X; Y = value.Y; }
	   }

	   public System.Drawing.Size Size
	   {
		 get { return new System.Drawing.Size(Width, Height); }
		 set { Width = value.Width; Height = value.Height; }
	   }

	   public static implicit operator System.Drawing.Rectangle(RECT r)
	   {
		 return new System.Drawing.Rectangle(r.Left, r.Top, r.Width, r.Height);
	   }

	   public static implicit operator RECT(System.Drawing.Rectangle r)
	   {
		 return new RECT(r);
	   }

	   public static bool operator ==(RECT r1, RECT r2)
	   {
		 return r1.Equals(r2);
	   }

	   public static bool operator !=(RECT r1, RECT r2)
	   {
		 return !r1.Equals(r2);
	   }

	   public bool Equals(RECT r)
	   {
		 return r.Left == Left && r.Top == Top && r.Right == Right && r.Bottom == Bottom;
	   }

	   public override bool Equals(object obj)
	   {
		 if (obj is RECT)
		   return Equals((RECT)obj);
		 else if (obj is System.Drawing.Rectangle)
		   return Equals(new RECT((System.Drawing.Rectangle)obj));
		 return false;
	   }

	   public override int GetHashCode()
	   {
		 return ((System.Drawing.Rectangle)this).GetHashCode();
	   }

	   public override string ToString()
	   {
		 return string.Format(System.Globalization.CultureInfo.CurrentCulture, "{{Left={0},Top={1},Right={2},Bottom={3}}}", Left, Top, Right, Bottom);
	   }
	}

	public delegate IntPtr HookProc(int code, int wParam, ref MSLLHOOKSTRUCT lParam);
	
	public delegate void CreateThreadProc(IntPtr lpPara);
	
	public class HotCornerMessageFilter : IMessageFilter
	{
		private const int WM_HOTKEY = 0x0312;
		
		public bool PreFilterMessage(ref Message m)
		{
			if (m.Msg == WM_HOTKEY)
			{
				Application.Exit();
			}
			return false;
		}
	}
}