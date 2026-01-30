package main

import (
	"bufio"
	"flag"
	"fmt"
	"os"
	"regexp"
	"strconv"
	"strings"
	"time"

	"golang.org/x/crypto/ssh"
)

// HIDKey 表示一个按键 HID 信息
type HIDKey struct {
	Code     byte // HID key code
	Modifier byte // Modifier: 0x02=Left Shift, 0x01=Left Ctrl, 0x04=Left Alt
}

var charToHID = map[rune]HIDKey{
	'a': {0x04, 0x00}, 'b': {0x05, 0x00}, 'c': {0x06, 0x00}, 'd': {0x07, 0x00},
	'e': {0x08, 0x00}, 'f': {0x09, 0x00}, 'g': {0x0A, 0x00}, 'h': {0x0B, 0x00},
	'i': {0x0C, 0x00}, 'j': {0x0D, 0x00}, 'k': {0x0E, 0x00}, 'l': {0x0F, 0x00},
	'm': {0x10, 0x00}, 'n': {0x11, 0x00}, 'o': {0x12, 0x00}, 'p': {0x13, 0x00},
	'q': {0x14, 0x00}, 'r': {0x15, 0x00}, 's': {0x16, 0x00}, 't': {0x17, 0x00},
	'u': {0x18, 0x00}, 'v': {0x19, 0x00}, 'w': {0x1A, 0x00}, 'x': {0x1B, 0x00},
	'y': {0x1C, 0x00}, 'z': {0x1D, 0x00},
	'A': {0x04, 0x02}, 'B': {0x05, 0x02}, 'C': {0x06, 0x02}, 'D': {0x07, 0x02},
	'E': {0x08, 0x02}, 'F': {0x09, 0x02}, 'G': {0x0A, 0x02}, 'H': {0x0B, 0x02},
	'I': {0x0C, 0x02}, 'J': {0x0D, 0x02}, 'K': {0x0E, 0x02}, 'L': {0x0F, 0x02},
	'M': {0x10, 0x02}, 'N': {0x11, 0x02}, 'O': {0x12, 0x02}, 'P': {0x13, 0x02},
	'Q': {0x14, 0x02}, 'R': {0x15, 0x02}, 'S': {0x16, 0x02}, 'T': {0x17, 0x02},
	'U': {0x18, 0x02}, 'V': {0x19, 0x02}, 'W': {0x1A, 0x02}, 'X': {0x1B, 0x02},
	'Y': {0x1C, 0x02}, 'Z': {0x1D, 0x02},
	'1': {0x1E, 0x00}, '2': {0x1F, 0x00}, '3': {0x20, 0x00}, '4': {0x21, 0x00},
	'5': {0x22, 0x00}, '6': {0x23, 0x00}, '7': {0x24, 0x00}, '8': {0x25, 0x00},
	'9': {0x26, 0x00}, '0': {0x27, 0x00},
	'-': {0x2D, 0x00}, '=': {0x2E, 0x00}, '[': {0x2F, 0x00}, ']': {0x30, 0x00},
	'\\': {0x31, 0x00}, ';': {0x33, 0x00}, '\'': {0x34, 0x00}, ',': {0x36, 0x00},
	'.': {0x37, 0x00}, '/': {0x38, 0x00}, '`': {0x35, 0x00}, ' ': {0x2C, 0x00},
	'!': {0x1E, 0x02}, '@': {0x1F, 0x02}, '#': {0x20, 0x02}, '$': {0x21, 0x02},
	'%': {0x22, 0x02}, '^': {0x23, 0x02}, '&': {0x24, 0x02}, '*': {0x25, 0x02},
	'(': {0x26, 0x02}, ')': {0x27, 0x02}, '_': {0x2D, 0x02}, '+': {0x2E, 0x02},
	'{': {0x2F, 0x02}, '}': {0x30, 0x02}, '|': {0x31, 0x02}, ':': {0x33, 0x02},
	'"': {0x34, 0x02}, '<': {0x36, 0x02}, '>': {0x37, 0x02}, '?': {0x38, 0x02},
	'~': {0x35, 0x02},
}

var specialKeys = map[string]HIDKey{
	"<F1>": {0x3A, 0x00}, "<F2>": {0x3B, 0x00}, "<F3>": {0x3C, 0x00},
	"<F4>": {0x3D, 0x00}, "<F5>": {0x3E, 0x00}, "<F6>": {0x3F, 0x00},
	"<F7>": {0x40, 0x00}, "<F8>": {0x41, 0x00}, "<F9>": {0x42, 0x00},
	"<F10>": {0x43, 0x00}, "<F11>": {0x44, 0x00}, "<F12>": {0x45, 0x00},
	"<ESC>": {0x29, 0x00}, "<TAB>": {0x2B, 0x00}, "<ENTER>": {0x28, 0x00},
	"<RETURN>": {0x28, 0x00}, "<BACKSPACE>": {0x2A, 0x00}, "<SPACE>": {0x2C, 0x00},
	"<INSERT>": {0x49, 0x00}, "<DELETE>": {0x4C, 0x00}, "<HOME>": {0x4A, 0x00},
	"<END>": {0x4D, 0x00}, "<PAGEUP>": {0x4B, 0x00}, "<PAGEDOWN>": {0x4E, 0x00},
	"<PGUP>": {0x4B, 0x00}, "<PGDN>": {0x4E, 0x00},
	"<UP>": {0x52, 0x00}, "<DOWN>": {0x51, 0x00}, "<LEFT>": {0x50, 0x00}, "<RIGHT>": {0x4F, 0x00},
	"<PRTSCR>": {0x46, 0x00}, "<PRINT>": {0x46, 0x00}, "<SCRLK>": {0x47, 0x00}, "<PAUSE>": {0x48, 0x00},
	"<LCTRL>": {0xE0, 0x00}, "<LSHIFT>": {0xE1, 0x00}, "<LALT>": {0xE2, 0x00}, "<LWIN>": {0xE3, 0x00},
	"<RCTRL>": {0xE4, 0x00}, "<RSHIFT>": {0xE5, 0x00}, "<RALT>": {0xE6, 0x00}, "<RWIN>": {0xE7, 0x00},
	"<CTRL_C>": {0x06, 0x01}, "<CTRL_V>": {0x19, 0x01}, "<CTRL_A>": {0x04, 0x01},
}

var seqNum uint16 = 0x0099

func calculateChecksum(modifier, code byte) byte {
	return 0xbe + modifier + code
}

func toHexString(data []byte) string {
	var sb strings.Builder
	for _, b := range data {
		sb.WriteString(fmt.Sprintf("\\x%02x", b))
	}
	return sb.String()
}

func buildHIDPacket(seqNum uint16, modifier, code byte) []byte {
	checksum := calculateChecksum(modifier, code)

	if checksum == 0xCC {
		packet := make([]byte, 17)
		packet[0] = 0xaa
		packet[1] = 0x0d
		packet[2] = 0x00
		packet[3] = byte(seqNum & 0xFF)
		packet[4] = byte((seqNum >> 8) & 0xFF)
		packet[5] = checksum
		packet[6] = 0x01
		packet[7] = 0x00
		packet[8] = 0x03
		packet[9] = modifier
		packet[10] = code
		packet[16] = 0xbb
		return packet
	}

	packet := make([]byte, 16)
	packet[0] = 0xaa
	packet[1] = 0x0d
	packet[2] = 0x00
	packet[3] = byte(seqNum & 0xFF)
	packet[4] = byte((seqNum >> 8) & 0xFF)
	packet[5] = checksum
	packet[6] = 0x00
	packet[7] = 0x03
	packet[8] = modifier
	packet[9] = code
	packet[15] = 0xbb
	return packet
}

type KeyAction struct {
	hid   HIDKey
	label string
	sleep time.Duration
}

func sendAllInOneSession(client *ssh.Client, keys []KeyAction, startSeq uint16) uint16 {
	seq := startSeq
	session, err := client.NewSession()
	if err != nil {
		fmt.Printf("❌ 创建 Session 失败: %v\n", err)
		return seq
	}
	defer session.Close()

	// 获取管道
	stdin, err := session.StdinPipe()
	if err != nil {
		fmt.Printf("❌ 获取 StdinPipe 失败: %v\n", err)
		return seq
	}
	defer stdin.Close()

	// 回显远端输出
	session.Stdout = os.Stdout
	session.Stderr = os.Stderr

	// 启动 Shell
	if err := session.Shell(); err != nil {
		fmt.Printf("❌ 启动 Shell 失败: %v\n", err)
		return seq
	}

	// 把命令写入 SSH stdin，同时打印到本地终端用于调试
	sendCmd := func(cmd string) {
		fmt.Printf("%s", cmd)
		if _, err := fmt.Fprint(stdin, cmd); err != nil {
			fmt.Printf("⚠ 写入 SSH stdin 失败: %v\n", err)
		}
	}

	fmt.Printf("批量发送 %d 个动作（单 Session 模式）...\n", len(keys))

	// 通过管道发送所有命令
	for i, key := range keys {
		if key.sleep > 0 {
			fmt.Printf("延时 %v...\n", key.sleep)
			time.Sleep(key.sleep)
			continue
		}
		// 按下
		pressData := buildHIDPacket(seq, key.hid.Modifier, key.hid.Code)
		seq++
		pressStr := toHexString(pressData)

		// 弹起
		releaseData := buildHIDPacket(seq, 0x00, 0x00)
		seq++
		releaseStr := toHexString(releaseData)

		// 写入命令
		sendCmd(fmt.Sprintf("printf '%s' > /dev/ttyAMA1\n", pressStr))
		time.Sleep(20 * time.Millisecond)
		sendCmd(fmt.Sprintf("printf '%s' > /dev/ttyAMA1\n", releaseStr))
		time.Sleep(20 * time.Millisecond)

		if i > 0 && i%10 == 0 {
			fmt.Printf("进度: %d/%d\n", i, len(keys))
		}
	}

	// 关闭输入，等待结束
	stdin.Close()
	session.Wait()
	//fmt.Println("✅ 全部发送完成")
	return seq
}

func parseInput(input string) []KeyAction {
	var result []KeyAction
	specialKeyRegex := regexp.MustCompile(`<[^>]+>`)
	pos := 0

	for pos < len(input) {
		if match := specialKeyRegex.FindStringIndex(input[pos:]); match != nil && match[0] == 0 {
			keyName := input[pos : pos+match[1]]
			upperKey := strings.ToUpper(strings.TrimSpace(keyName))
			if strings.HasPrefix(upperKey, "<SLEEP") && strings.HasSuffix(upperKey, ">") {
				payload := strings.TrimSpace(strings.TrimSuffix(strings.TrimPrefix(upperKey, "<SLEEP"), ">"))
				seconds, err := strconv.Atoi(payload)
				if err != nil || seconds < 0 {
					fmt.Printf("⚠ 无效延时指令: %s\n", keyName)
				} else {
					result = append(result, KeyAction{sleep: time.Duration(seconds) * time.Second, label: keyName})
				}
				pos += match[1]
				continue
			}
			if hid, ok := specialKeys[keyName]; ok {
				result = append(result, KeyAction{hid: hid, label: keyName})
				pos += match[1]
				continue
			}
		}

		r := rune(input[pos])
		if hid, ok := charToHID[r]; ok {
			result = append(result, KeyAction{hid: hid, label: string(r)})
		} else {
			fmt.Printf("⚠ 跳过未知字符: %c (0x%04X)\n", r, r)
		}
		pos++
	}
	return result
}

func main() {
	var (
		host     string
		username string
		password string
		showHelp bool
	)

	flag.StringVar(&host, "host", "", "控控IP:SSH端口")
	flag.StringVar(&username, "user", "admin", "SSH登录用户名（可选，默认为admin）")
	flag.StringVar(&password, "password", "", "SSH登录密码（可选，未提供则会要求单独输入）")
	flag.BoolVar(&showHelp, "h", false, "显示帮助")
	flag.BoolVar(&showHelp, "help", false, "显示帮助")
	flag.Usage = func() {
		fmt.Fprintf(flag.CommandLine.Output(), "\n用法: %s [options]\n\n", os.Args[0])
		fmt.Fprintln(flag.CommandLine.Output(), "选项:")
		flag.PrintDefaults()
		fmt.Fprintln(flag.CommandLine.Output(), "\n示例:")
		fmt.Fprintf(flag.CommandLine.Output(), "  %s -host 192.168.1.232:44022 -password '{D!oF=Xu+@!.'\n", os.Args[0])
	}
	flag.Parse()

	if showHelp {
		flag.Usage()
		return
	}

	if strings.TrimSpace(host) == "" {
		fmt.Println("❌ 缺少主机参数 -host ip:端口")
		flag.Usage()
		return
	}

	if strings.TrimSpace(password) == "" {
		fmt.Print("请输入SSH登录密码: ")
		pwdReader := bufio.NewReader(os.Stdin)
		inputPwd, _ := pwdReader.ReadString('\n')
		password = strings.TrimSpace(inputPwd)
		if password == "" {
			fmt.Println("❌ 密码不能为空")
			return
		}
	}

	sshConfig := &ssh.ClientConfig{
		User:            username,
		Auth:            []ssh.AuthMethod{ssh.Password(password)},
		HostKeyCallback: ssh.InsecureIgnoreHostKey(),
		Timeout:         5 * time.Second,
	}

	fmt.Printf("连接 %s ...\n", host)

	client, err := ssh.Dial("tcp", host, sshConfig)
	if err != nil {
		fmt.Printf("❌ 连接失败: %v\n", err)
		return
	}
	defer client.Close()
	fmt.Println("✅ 连接成功")

	reader := bufio.NewReader(os.Stdin)

	fmt.Println("HID 键盘控制器")
	fmt.Println("支持格式：")
	fmt.Println("  - 普通文本：直接输入字符")
	fmt.Println("  - 特殊按键：<F1>-<F12>, <ESC>, <TAB>, <ENTER>, <BACKSPACE>")
	fmt.Println("  - 导航键：<UP>, <DOWN>, <LEFT>, <RIGHT>, <HOME>, <END>")
	fmt.Println("  - 功能键：<INSERT>, <DELETE>, <PAGEUP>, <PAGEDOWN>")
	fmt.Println("  - 组合键：<CTRL_C>, <CTRL_V>, <CTRL_A>")
	fmt.Println("  - 延时指令：<SLEEP N> (N为秒数)")
	fmt.Println("示例：Hello<F1><ENTER><CTRL_A>")
	fmt.Println("输入 <QUIT> 退出（或 Ctrl+C 终止）")

	for {
		fmt.Print("\n请输入要发送的字符串: ")
		text, err := reader.ReadString('\n')
		if err != nil {
			fmt.Printf("读取输入失败: %v\n", err)
			return
		}
		text = strings.TrimSpace(text)

		if strings.EqualFold(text, "<QUIT>") {
			fmt.Println("收到 <QUIT>，退出")
			return
		}

		if text == "" {
			fmt.Println("空输入，跳过")
			continue
		}

		keys := parseInput(text)
		if len(keys) == 0 {
			fmt.Println("无有效按键，跳过")
			continue
		}

		start := time.Now()
		seqNum = sendAllInOneSession(client, keys, seqNum)
		//sendBatch(client, keys, seqNum)

		fmt.Printf("✅ 发送完成! 本次耗时: %v\n", time.Since(start))
	}
}
