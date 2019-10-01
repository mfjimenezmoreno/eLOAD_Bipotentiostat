from bokeh.layouts import widgetbox
from bokeh.models import CustomJS, TextInput, Paragraph
from bokeh.plotting import output_file, show

# SAVE
output_file('Sample_Application.html', mode='inline', root_dir=None)

# PREP DATA
welcome_message = 'You have selected: (none)'

# TAKE ONLY OUTPUT
text_banner = Paragraph(text=welcome_message, width=200, height=100)

# CALLBACKS


def callback_print(text_banner=text_banner):
    user_input = str(cb_obj.value)
    welcome_message = 'You have selected: ' + user_input
    text_banner.text = welcome_message


# USER INTERACTIONS
text_input = TextInput(value="", title="Enter row number:",
                       callback=CustomJS.from_py_func(callback_print))

# LAYOUT
widg = widgetbox(text_input, text_banner)
show(widg)
