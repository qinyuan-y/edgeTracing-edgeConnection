import tkinter as tk
from tkinter import ttk
import config_parser
import pathlib

class MainApplication(tk.Frame):

    def __init__(self, parent, *args, **kwargs):
        tk.Frame.__init__(self, parent, *args, **kwargs)
        self.parent = parent
        top_frame = tk.Frame(root)
        self.canvas = tk.Canvas(top_frame, width=300, height=300, bd=0)
        self.canvas.bind('<Button-1>', self.clicked)

        # Load config or generate new one if not existing
        self.config_file = pathlib.Path(__file__).parent / 'config.csv'
        self.data = []
        if self.config_file.is_file():
            self.data = config_parser.ConfigParser.load_config(self.config_file)
        else:
            self.data = config_parser.ConfigParser.load_config(
                config_file = pathlib.Path(__file__).parent / 'default_config.csv')

        # combobox to choose 3x3 combination
        self.cbox = ttk.Combobox(root, values=[])
        self.cbox.bind("<<ComboboxSelected>>", self.update_view)

        ttk.Button(top_frame, text="<", width=3,
                   command=self.prev_combination).pack(side=tk.LEFT, padx=5)

        # create canvas for 3x3 matrix
        self.canvas.pack(side=tk.LEFT, padx=10, pady=5)
        ttk.Button(top_frame, text=">", width=3,
                   command=self.next_combination).pack(side=tk.RIGHT, padx=5)
        self.rotate = tk.IntVar()
        top_frame.pack(padx=10, pady=5)
        self.color = ["red", "green", "sky blue", "cyan",
                      "yellow", "magenta", "orange", "pink"]        # add color for more edges
        self.current_edge = 0
        self.ids = [""]

        # set global styling
        style = ttk.Style(root)
        style.theme_use('clam')

        # Display combobox
        self.cbox.pack(fill="x", padx=15, pady=5)

        # Edge information
        info_frame = tk.Frame(root)
        self.rot1 = tk.StringVar()
        self.rot2 = tk.StringVar()
        self.rot3 = tk.StringVar()
        self.rot4 = tk.StringVar()
        ttk.Label(info_frame, textvariable=self.rot1).pack(side=tk.LEFT)
        ttk.Label(info_frame, textvariable=self.rot2).pack(side=tk.LEFT)
        ttk.Label(info_frame, textvariable=self.rot3).pack(side=tk.LEFT)
        ttk.Label(info_frame, textvariable=self.rot4).pack(side=tk.LEFT)
        info_frame.pack()

        # first button row
        btn_frame_1 = tk.Frame(root)
        ttk.Button(btn_frame_1, text="previous edge",
                   command=self.prev_edge).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame_1, text="next edge",
                   command=self.next_edge).pack(side=tk.LEFT, padx=5)

        # information about current id
        self.id_var = tk.StringVar()
        self.id_var.set(self.color[0])
        ttk.Label(btn_frame_1, textvariable=self.id_var).pack(
            side=tk.RIGHT, padx=5)
        ttk.Label(btn_frame_1, text="edge color:").pack(side=tk.RIGHT)
        btn_frame_1.pack(fill="x", padx=10, pady=5)

        # third button row
        btn_frame_2 = tk.Frame(root)
        ttk.Button(btn_frame_2, text="reset edge",
                   command=self.reset_edge).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame_2, text="reset all edges",
                   command=self.reset_cfg).pack(side=tk.LEFT, padx=5)
        ttk.Button(btn_frame_2, text="save config",
                   command=self.save_config).pack(side=tk.RIGHT, padx=5)
        ttk.Checkbutton(btn_frame_2, text="rot. invariance", command=self.update_view,
                        variable=self.rotate).pack(side=tk.LEFT, padx=5)
        btn_frame_2.pack(fill="x", padx=10, pady=5)

        self.indices = []
        self.get_indices()
        self.cbox.current(0)
        self.update_view()

    def save_config(self):
        output_data = [[] for i in range(255)]
        shift_map = [6, 3, 0, 7, 4, 1, 8, 5, 2]
        for i in self.indices:
            if self.rotate.get():
                rotated_config = self.data[i]
                output_data[i] = rotated_config
                # save for all rotations
                for steps in range(2, 8, 2):
                    shifted_binary = self.shift_value(i, steps)
                    rotated_config = [''.join([i.replace(i, str(shift_map[int(i)])) for i in edge])
                                      for edge in rotated_config if edge != '']
                    output_data[shifted_binary] = rotated_config
            else:
                config_line = [''.join(map(str, edge))
                               for edge in self.data[i] if edge != '']
                output_data[i] = config_line
        config_parser.ConfigParser.save_config(output_data, self.config_file)

    def update_view(self, event=None):
        self.get_indices()
        if self.data[int(self.cbox.get())] != []:
            self.ids = self.data[int(self.cbox.get())]
        else:
            self.ids = [""]
        seq = int(self.cbox.get())
        self.rot1.set(str(seq) + ":" + f"{seq:08b}")
        if self.rotate.get():
            seq = self.shift_value(seq, 2)
            self.rot2.set(str(seq) + ":" + f"{seq:08b}")
            seq = self.shift_value(seq, 2)
            self.rot3.set(str(seq) + ":" + f"{seq:08b}")
            seq = self.shift_value(seq, 2)
            self.rot4.set(str(seq) + ":" + f"{seq:08b}")
        else:
            self.rot2.set("")
            self.rot3.set("")
            self.rot4.set("")
        self.drawRect()

    def clicked(self, event):
        binary = f"{int(self.cbox.get()):08b}"
        binary += '1'
        map_bin = [0, 1, 2, 7, 8, 3, 6, 5, 4]
        binary = [binary[index] for index in map_bin]
        grid_number = (int(event.y / 101) * 3) + (int(event.x / 101))
        if binary[grid_number] != '0':  # do nothing for black pixel
            if len(self.ids[self.current_edge]) == 0:
                if str(grid_number) not in self.ids[self.current_edge]:
                    self.ids[self.current_edge] += (str(grid_number))
            else:
                last_val = int(self.ids[self.current_edge][-1])
                distance = ((int(grid_number/3)-int(last_val/3)) **
                            2 + (grid_number % 3 - last_val % 3)**2)**0.5
                if str(grid_number) not in self.ids[self.current_edge] and distance <= 2**0.5:
                    self.ids[self.current_edge] += (str(grid_number))
                elif str(grid_number) == self.ids[self.current_edge][-1]:
                    self.ids[self.current_edge] = self.ids[self.current_edge][:-1]
        # Update visuals and data
        self.drawRect()
        self.data[int(self.cbox.get())] = self.ids

    def drawRect(self):
        binary = f"{int(self.cbox.get()):08b}"
        binary += '1'
        map_bin = [0, 1, 2, 7, 8, 3, 6, 5, 4]
        binary = [binary[index] for index in map_bin]
        for index in range(len(binary)):
            if (index == 0):
                x, y = 0, 0
            elif (index == 1):
                x, y = 100, 0
            elif (index == 2):
                x, y = 200, 0
            elif (index == 3):
                x, y = 0, 100
            elif (index == 4):
                x, y = 100, 100
            elif (index == 5):
                x, y = 200, 100
            elif (index == 6):
                x, y = 0, 200
            elif (index == 7):
                x, y = 100, 200
            elif (index == 8):
                x, y = 200, 200

            if binary[index] == '1':
                self.canvas.create_rectangle(
                    x, y, x + 100, y + 100, fill='white', outline='white')
                indices = []
                for i in range(len(self.ids)):
                    if str(index) in self.ids[i]:
                        indices.append(i)
                l = len(indices)
                for j in range(len(indices)):
                    self.canvas.create_rectangle(x, y + (100/l * j), x + 100, y + (100/l * j) + 100/l,
                                                 fill=self.color[indices[j]], outline='white')
                    self.canvas.create_text(x + 50, y + (100/l * j) + 50/l, font=('Arial', 12, 'bold'),
                                            fill='black', text=self.ids[indices[j]].index(str(index)))

            else:
                self.canvas.create_rectangle(
                    x, y, x + 100, y + 100, fill='black', outline='white')

    def next_edge(self):
        if self.current_edge < 7:
            if len(self.ids[self.current_edge]) == 0 and self.current_edge > 0:
                self.ids.remove('')
                if not (self.current_edge == len(self.ids)-1):
                    self.current_edge -= 1
            self.current_edge += 1
            if self.current_edge + 1 > len(self.ids):
                self.ids.append('')

        self.id_var.set(self.color[self.current_edge])
        self.drawRect()

    def prev_edge(self):
        if len(self.ids[self.current_edge]) == 0 and self.current_edge > 0:
            self.ids.remove('')
            self.current_edge -= 1
        elif (self.current_edge > 0):
            self.current_edge -= 1
        self.id_var.set(self.color[self.current_edge])
        self.drawRect()

    def reset_edge(self):
        self.ids[self.current_edge] = ''
        self.drawRect()

    def reset_cfg(self):
        self.ids = [""]
        self.current_edge = 0
        self.id_var.set(self.color[self.current_edge])
        self.drawRect()

    def shift_value(self, value, steps):
        shifted = value << steps
        overflowed = (shifted) >> 8
        shifted &= 255
        return overflowed | shifted

    def next_combination(self):
        if int(self.cbox.get()) in self.indices:
            index = self.indices.index(int(self.cbox.get()))
            if (index < len(self.indices) - 1):
                self.cbox.current(index+1)
                self.update_view()
                self.current_edge = 0
                self.id_var.set(self.color[self.current_edge])
        else:
            self.cbox.current(0)
            index = 0
            self.update_view()
            self.current_edge = 0
            self.id_var.set(self.color[self.current_edge])

    def prev_combination(self):
        if int(self.cbox.get()) in self.indices:
            index = self.indices.index(int(self.cbox.get()))
            if (index > 0):
                self.cbox.current(index-1)
                self.update_view()
                self.current_edge = 0
                self.id_var.set(self.color[self.current_edge])
        else:
            self.cbox.current(0)
            index = 0
            self.update_view()
            self.current_edge = 0
            self.id_var.set(self.color[self.current_edge])

    def get_indices(self, event=None):
        checked_cases = []
        self.indices = []
        for i in range(0, 256):
            found = False
            ones = True if '111' not in f"{i:08b}"+f"{i:08b}" else False
            if self.rotate.get():
                for steps in range(0, 8, 2):
                    shifted_i = self.shift_value(i, steps)
                    if shifted_i in checked_cases:
                        found = True
                        break
            if(bin(i).count("1")) > 2 and ones and not found:
                checked_cases.append(i)

        # Find all cases
        for i in checked_cases:
            count = 0
            b = f"{i:08b}"
            for j in range(0, 8):
                if b[j] == '1':
                    if (j % 2):
                        count += 1
                    else:
                        next_neighbor = b[1+j] if j < 7 else b[0]
                        prev_neighbor = b[j-1] if j > 0 else b[7]
                        if next_neighbor == '0' and prev_neighbor == '0':
                            count += 1
            if count > 2:
                self.indices.append(i)
        self.cbox["values"] = self.indices


if __name__ == "__main__":
    root = tk.Tk()
    root.title("Config Tool")
    MainApplication(root).pack(side="top", fill="both", expand=True)
    root.mainloop()
