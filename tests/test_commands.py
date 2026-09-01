import unittest

from commands import parse_command


class ParseCommandTests(unittest.TestCase):
    def test_parses_preset_by_word(self):
        self.assertEqual(
            parse_command("ativa a brisa"),
            [{"type": "preset", "name": "brisa"}],
        )

    def test_parses_brightness(self):
        self.assertEqual(
            parse_command("brilho de 80"),
            [{"type": "brightness", "value": 80}],
        )

    def test_parses_preset_and_brightness(self):
        self.assertEqual(
            parse_command("brisa brilho 80"),
            [
                {"type": "preset", "name": "brisa"},
                {"type": "brightness", "value": 80},
            ],
        )

    def test_end_session_has_priority(self):
        self.assertEqual(parse_command("encerrar brisa"), [{"type": "end_session"}])

    def test_rejects_out_of_range_brightness(self):
        self.assertEqual(parse_command("brilho 256"), [])


if __name__ == "__main__":
    unittest.main()
