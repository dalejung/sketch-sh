open Editor_Types;
module Block = Editor_Types.Block;

module V1 = {
  module JsonDecode = {
    open Json.Decode;
    let blockDataDecoder: Js.Json.t => Block.blockData =
      json => {
        let kind = json |> field("kind", string);
        let value = json |> field("value", string);
        switch (kind) {
        | "code" =>
          B_Code({bc_value: value, bc_firstLineNumber: 1, bc_widgets: [||]})
        | "text" => B_Text(value)
        | _ => raise(Invalid_argument("Unexpected block type: " ++ kind))
        };
      };

    let blockDecoder: Js.Json.t => Block.block =
      json => {
        b_id: json |> field("id", string),
        b_data: json |> field("data", blockDataDecoder),
        b_deleted:
          json
          |> optional(field("deleted", bool))
          |> (
            fun
            | None => false
            | Some(bool) => bool
          ),
      };
    let langDecoder: Js.Json.t => lang =
      json => json |> string |> stringToLang;
    let decode: Js.Json.t => (lang, array(Block.block)) =
      json => (
        json
        |> optional(field("lang", langDecoder))
        |> (
          fun
          | None => RE
          | Some(lang) => lang
        ),
        json |> field("blocks", array(blockDecoder)),
      );
  };

  module JsonEncode = {
    open Json.Encode;

    let blockDataEncoder: Block.blockData => Js.Json.t =
      fun
      | B_Code({bc_value}) =>
        object_([("kind", string("code")), ("value", string(bc_value))])
      | B_Text(value) =>
        object_([("kind", string("text")), ("value", string(value))]);

    let blockEncoder: Block.block => Js.Json.t =
      ({b_id, b_data, b_deleted}) =>
        object_([
          ("id", string(b_id)),
          ("data", blockDataEncoder(b_data)),
          ("deleted", bool(b_deleted)),
        ]);

    let encode: (lang, array(Block.block)) => Js.Json.t =
      (lang, blocks) =>
        object_([
          ("lang", lang |> langToString |> string),
          (
            "blocks",
            blocks
            ->(Belt.Array.mapU((. block) => block |> blockEncoder))
            ->jsonArray,
          ),
        ]);
  };
  let decode = JsonDecode.decode;
  let encode = JsonEncode.encode;
};
