#!/usr/bin/env python3
"""Offline generator regression checks; never connects to a database."""
import argparse
import os
from pathlib import Path
import shlex
import subprocess
import tempfile
import unittest

ROOT = Path(__file__).resolve().parents[1]

class GeneratorTests(unittest.TestCase):
    @classmethod
    def setUpClass(cls):
        cls.temp = tempfile.TemporaryDirectory()
        cls.base = Path(cls.temp.name)
        cls.exe = cls.base / 'ccObjMaker'
        command = shlex.split(os.environ.get('CC', 'gcc'))
        command += ['-Wall', '-Wextra', '-Werror', '-Wformat=2', '-o', str(cls.exe), str(ROOT / 'ccObjMaker.c')]
        if ARGS.sanitize:
            command += ['-g', '-fsanitize=address,undefined']
        subprocess.run(command, check=True)
        cls.counter = 0

    @classmethod
    def tearDownClass(cls):
        cls.temp.cleanup()

    def generate(self, text, success=True):
        type(self).counter += 1
        work = self.base / str(self.counter)
        work.mkdir()
        (work / 'fields.csv').write_bytes(text.encode())
        result = subprocess.run([str(self.exe), 'Sample', 'fields.csv'], cwd=work, capture_output=True, text=True)
        if success:
            self.assertEqual(result.returncode, 0, result.stderr)
            self.assertEqual(result.stderr, '')
        else:
            self.assertNotEqual(result.returncode, 0)
            self.assertFalse((work / 'Sample.cs').exists())
        return work, result

    def test_line_endings_and_no_final_newline(self):
        for ending in ['\n', '\r\n', '']:
            with self.subTest(ending=ending):
                work, _ = self.generate('Count; INT ; DEFAULT 0 ;0;' + ending)
                self.assertIn('protected int _Count;', (work / 'Sample.cs').read_text())

    def test_output_contracts(self):
        cases = [
            'Count;INT;DEFAULT 0;0;\nAmount;DECIMAL(12,3);DEFAULT 0;0;\nDescription;VARCHAR(100);NULL;0;\n',
            'Secret;varbinary(255);NULL;0;\nSecondSecret;VARBINARY(255);NULL;0;\n',
            'OnDate;DATE;NULL;0;\nCount;INT;DEFAULT 0;0;\nRatio;DOUBLE;DEFAULT 0;0;\nAtTime;TIME;NULL;0;\n',
        ]
        for text in cases:
            with self.subTest(text=text):
                work, _ = self.generate(text)
                cs = (work / 'Sample.cs').read_text()
                form = (work / 'Sample.frm.cs').read_text()
                mysql = (work / 'Sample.mysql').read_text()
                self.assertNotIn('\n,string whereString=', cs)
                self.assertNotIn('io.InspectionId', cs)
                self.assertNotIn('NBNull', cs)
                self.assertIn('cmd.Transaction = tx;', cs)
                self.assertIn('cmd.CommandText = "sp_UpdateSample";', cs)
                self.assertIn('if(SortBy.Length>0) sortString +=', cs)
                self.assertIn('o.SampleID', form)
                self.assertNotIn('b.load(o.Sample,', form)
                self.assertNotIn('.loadSample(', form)
                self.assertNotIn('cOld', form)
                self.assertIn('DROP PROCEDURE IF EXISTS sp_updateSample //', mysql)
                self.assertEqual(mysql.count('DELIMITER ;'), 2)
                if 'VARBINARY' in text.upper():
                    self.assertIn('public string Secret', cs)
                    self.assertIn('if(SortBy=="Secret")', cs)
                    self.assertIn('o = load(ID, con, o,key);', cs)
                    self.assertIn('SortBy, SortDirection,key,whereString);', cs)
                    self.assertIn('SET @key = @encryptionKey', cs)
                    self.assertNotIn('SET @key:=', cs)
                    self.assertNotIn('strFields += ",setEDataHKDF', cs)
                else:
                    mysql_section = cs.split('/// SQL Server Version')[0]
                    self.assertNotIn('keyCommand', mysql_section)
                if 'DECIMAL' in text:
                    self.assertIn('protected double _Amount;', cs)
                    self.assertIn('o.Amount =r.IsDBNull("Amount") ? 0 : r.GetDouble("Amount");', cs)
                    self.assertIn('double.TryParse', form)
                if 'DATE' in text:
                    self.assertIn('DateTime.TryParse(this.txtOnDate.Text, out DateTime valueOnDate)', form)
                    self.assertIn('int.TryParse(this.txtCount.Text, out int valueCount)', form)
                subprocess.run(shlex.split(os.environ.get('CXX', 'g++')) + ['-std=c++11', '-x', 'c++', '-fsyntax-only', str(work / 'Sample.h')], check=True)
                if ARGS.csharp_checker:
                    subprocess.run(shlex.split(ARGS.csharp_checker) + [str(work)], check=True)

    def test_invalid_inputs_fail_before_output(self):
        cases = ['', '\n', 'OnlyOneColumn\n', 'X;INT;NULL\nX;INT;NULL\n',
                 'Bad Name;INT;NULL\n', 'class;INT;NULL\n', 'RecordDeleted;INT;NULL\n',
                 'X' * 128 + ';INT;NULL\n', 'X;INT;NULL;0;;extra\n',
                 ''.join(f'Field{i};INT;NULL\n' for i in range(129))]
        for text in cases:
            with self.subTest(text=text):
                _, result = self.generate(text, False)
                self.assertTrue(result.stderr)

    def test_128_fields(self):
        self.generate(''.join(f'Field{i};INT;NULL\n' for i in range(128)))

    def test_usage(self):
        for args in [[], ['Sample']]:
            result = subprocess.run([str(self.exe)] + args, capture_output=True, text=True)
            self.assertNotEqual(result.returncode, 0)

if __name__ == '__main__':
    parser = argparse.ArgumentParser()
    parser.add_argument('--sanitize', action='store_true')
    parser.add_argument('--csharp-checker', help='Command for the optional compiled CSharpChecks tool')
    ARGS, rest = parser.parse_known_args()
    unittest.main(argv=[__file__] + rest)
