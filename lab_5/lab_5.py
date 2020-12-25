import smtplib
import sys
import argparse
from os.path import isfile, join, basename, splitext
from os import listdir
from email.mime.application import MIMEApplication
from email.mime.multipart import MIMEMultipart
from email.mime.text import MIMEText

def find_files(keyword):
    all_files = [f for f in listdir(".") if isfile(join(".", f))]
    files_to_attach = []

    for file_name in all_files:
        file_extension = splitext(file_name)[1]

        if file_extension == '.txt':
            with open(file_name) as f:
                if keyword in f.read():
                    files_to_attach.append(file_name)

    return files_to_attach

def send_mail(opt, config):
    
    msg = MIMEMultipart()

    msg['From'] = opt.from_mail
    msg['To'] = opt.to_mail
    msg['Subject'] = "Description"

    body = "lab_5"
    msg.attach(MIMEText(body, 'plain'))

    if opt.keyword != None:
        files = find_files(opt.keyword)

        for f in files or []:
            with open(f, "rb") as fl:
                att = MIMEApplication(fl.read(), Name=basename(f))
            att['Content-Disposition'] = 'attachment; filename="%s"' % basename(f)
            msg.attach(att)

    server = smtplib.SMTP(config['host'], config['port'])
    server.starttls()
    server.login(opt.from_mail, opt.password)

    res = server.sendmail(msg['From'], msg['To'], msg.as_string())

    server.quit()

    print("Successfully sent email message to %s" % (msg['To']))

def get_args():
    parser = argparse.ArgumentParser()

    parser.add_argument('from_mail', action="store", help="Email address that the message is sent from")
    parser.add_argument('password', action="store", help="Password for the email address that the message is sent from")
    parser.add_argument('to_mail', action="store", help="Email address to which the message is sent")
    parser.add_argument('-k', '--keyword', action="store", required=False, help="Substring for attaching files")

    return parser

def main():
    parser = get_args()
    args = parser.parse_args(sys.argv[1:])

    config = { 'host': "smtp.mail.ru", 'port': 2525 }

    send_mail(args, config)

if __name__ == '__main__':
    main()

